/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024 M. Frohne
//
//  Casil is free software: you can redistribute it and/or modify it
//  under the terms of the GNU Affero General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Casil is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty
//  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with Casil. If not, see <https://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef CASIL_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H
#define CASIL_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/error_code.hpp>

#include <chrono>
#include <cstddef>
#include <concepts>
#include <functional>
#include <future>
#include <optional>
#include <stdexcept>

namespace casil
{

namespace TL
{

/*!
 * \brief Common implementations for the different Interface classes.
 */
namespace CommonImpl
{

/*!
 * \brief Helper functions for using the asynchronous functionality of the Boost %ASIO library.
 */
namespace ASIOHelper
{

/*!
 * \brief Check if type is either a Boost %ASIO %TCP socket or a Boost %ASIO %UDP socket.
 *
 * \tparam SocketT
 */
template<typename SocketT>
concept IsSocket = (std::same_as<SocketT, boost::asio::ip::tcp::socket> || std::same_as<SocketT, boost::asio::ip::udp::socket>);

/*!
 * \brief Check if a Boost %ASIO %TCP or %UDP socket has a \c cancel() function with \c void return type.
 *
 * \tparam SocketT
 */
template<typename SocketT>
concept IsCancellableSocket = IsSocket<SocketT> && requires(SocketT sock)
{
    { sock.cancel() } -> std::same_as<void>;    //This check is kind of paranoid
};

template<typename ReturnT, typename SocketT>
    requires IsCancellableSocket<SocketT>
ReturnT getAsyncBoostFutureWithTimedOutCancel(std::future<ReturnT>& pFuture, SocketT& pSocket, std::chrono::milliseconds pTimeout,
                                              const std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                            ///< \brief Wait for the future, get and return its value;
                                                                            ///  cancel socket and throw an exception on timeout.

template<typename SocketT>
    requires IsCancellableSocket<SocketT>
std::size_t getAsyncTransferredWithTimedOutCancel(std::promise<std::size_t>& pPromiseN, SocketT& pSocket, std::chrono::milliseconds pTimeout,
                                                  const std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                            ///< \brief Wait for the promised future, get and return its value;
                                                                            ///  cancel the socket on timeout but return future's value anyway.

void readWriteHandler(const boost::system::error_code& pErrorCode, std::size_t pNumBytes, std::promise<std::size_t>& pNumBytesPromise);
                                                                            ///< \brief Handler for socket transfer operations
                                                                            ///  that does not fail when the socket gets cancelled.


//Template function definitions


/*!
 * \brief Wait for the future, get and return its value; cancel socket and throw an exception on timeout.
 *
 * \todo Detailed doc
 *
 * \tparam ReturnT
 * \tparam SocketT
 * \param pFuture
 * \param pSocket
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
template<typename ReturnT, typename SocketT>
    requires IsCancellableSocket<SocketT>
ReturnT getAsyncBoostFutureWithTimedOutCancel(std::future<ReturnT>& pFuture, SocketT& pSocket, std::chrono::milliseconds pTimeout,
                                              const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pTimedOut.has_value())
        pTimedOut->get() = false;

    if (!pFuture.valid())
        throw std::runtime_error("Invalid future. THIS SHOULD NEVER HAPPEN!");

    const std::future_status status = pFuture.wait_for(pTimeout);

    if (status == std::future_status::ready)
    {
        return pFuture.get();   //This may throw an exception
    }
    else if (status == std::future_status::timeout)
    {
        pSocket.cancel();

        pFuture.wait();

        try
        {
            return pFuture.get();   //Return just in case future got ready before cancel; otherwise, Boost's use_future throws anyway
        }
        catch (const boost::system::system_error& exc)
        {
            if (exc.code().value() == boost::system::errc::operation_canceled)
            {
                if (pTimedOut.has_value())
                    pTimedOut->get() = true;

                throw std::runtime_error("Timeout.");
            }
            else
                throw;
        }
    }
    else
        throw std::runtime_error("Deferred future. THIS SHOULD NEVER HAPPEN!");
}

/*!
 * \brief Wait for the promised future, get and return its value; cancel the socket on timeout but return future's value anyway.
 *
 * \todo Detailed doc
 *
 * \tparam SocketT
 * \param pPromiseN
 * \param pSocket
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
template<typename SocketT>
    requires IsCancellableSocket<SocketT>
std::size_t getAsyncTransferredWithTimedOutCancel(std::promise<std::size_t>& pPromiseN, SocketT& pSocket, std::chrono::milliseconds pTimeout,
                                                  const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pTimedOut.has_value())
        pTimedOut->get() = false;

    std::future<std::size_t> futureN;

    try
    {
        futureN = pPromiseN.get_future();
    }
    catch (const std::future_error&)
    {
        throw std::invalid_argument("Invalid promise.");
    }

    const std::future_status status = futureN.wait_for(pTimeout);

    if (status == std::future_status::ready)
    {
        return futureN.get();   //This may throw an exception, depending on the used async handler
    }
    else if (status == std::future_status::timeout)
    {
        pSocket.cancel();

        futureN.wait();

        if (pTimedOut.has_value())
            pTimedOut->get() = true;

        return futureN.get();   //This may throw an exception, depending on the used async handler
    }
    else
        throw std::runtime_error("Deferred future. THIS SHOULD NEVER HAPPEN!");
}

} // namespace ASIOHelper

} // namespace CommonImpl

} // namespace TL

} // namespace casil

#endif // CASIL_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H
