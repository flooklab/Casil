/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2026 M. Frohne
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
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

#ifndef CASIL_LAYERS_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H
#define CASIL_LAYERS_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/serial_port.hpp>
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

namespace Layers::TL
{

/// \cond INTERNAL
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
 * \brief Check if type is a Boost %ASIO %TCP socket, a Boost %ASIO %UDP socket or a Boost %ASIO serial port.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept IsSocketOrSerPort = (std::same_as<T, boost::asio::ip::tcp::socket> ||
                             std::same_as<T, boost::asio::ip::udp::socket> ||
                             std::same_as<T, boost::asio::serial_port>);

/*!
 * \brief Check if type is a Boost %ASIO %TCP resolver or a Boost %ASIO %UDP resolver.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept IsResolver = (std::same_as<T, boost::asio::ip::tcp::resolver> ||
                      std::same_as<T, boost::asio::ip::udp::resolver>);

/*!
 * \brief Check if a Boost %ASIO %TCP or %UDP socket or serial port has a \c cancel() function with \c void return type.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept IsCancellableSocketOrSerPort = IsSocketOrSerPort<T> && requires(T sockPort)
{
    { sockPort.cancel() } -> std::same_as<void>;    //This check is kind of paranoid
};

/*!
 * \brief Check if a Boost %ASIO %TCP or %UDP resolver has a \c cancel() function with \c void return type.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept IsCancellableResolver = IsResolver<T> && requires(T resolver)
{
    { resolver.cancel() } -> std::same_as<void>;    //This check is kind of paranoid
};

template<typename ReturnT, typename T>
    requires (IsCancellableSocketOrSerPort<T> || IsCancellableResolver<T>)
ReturnT getAsyncBoostFutureWithTimedOutCancel(std::future<ReturnT>& pFuture, T& pSockPortResolver, std::chrono::milliseconds pTimeout,
                                              const std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                            ///< \brief Wait for the future, get and return its value; cancel
                                                                            ///  the socket/port/resolver and throw an exception on timeout.

template<typename T>
    requires IsCancellableSocketOrSerPort<T>
std::size_t getAsyncTransferredWithTimedOutCancel(std::promise<std::size_t>& pPromiseN, T& pSocketOrPort, std::chrono::milliseconds pTimeout,
                                                  const std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                            ///< \brief Wait for the promised future, get and return
                                                                            ///  its value; cancel the socket/port on timeout
                                                                            ///  but return future's value anyway.

void readWriteHandler(const boost::system::error_code& pErrorCode, std::size_t pNumBytes, std::promise<std::size_t>& pNumBytesPromise);
                                                                            ///< \brief Handler for socket/port transfer operations
                                                                            ///  that does not fail when the socket/port gets cancelled.


//Template function definitions


/*!
 * \brief Wait for the future, get and return its value; cancel the socket/port/resolver and throw an exception on timeout.
 *
 * This function should be used to retrieve the result of an asynchronous operation (in terms of the return value of the equivalent
 * synchronous call) on a socket/port/resolver \p pSockPortResolver using the built-in \c boost::asio::use_future handler when this
 * very result is not needed anymore if the operation times out. After \p pTimeout with still unfinished operation \p pSockPortResolver
 * will be cancelled. If the operation finishes before the timeout or during the cancelling of the socket/port/resolver,
 * the result will be returned. Otherwise an exception is thrown, in which case \p pTimedOut will be set to true (if defined).
 *
 * Note that \p pTimedOut is always set to false in the beginning, if defined.
 *
 * \throws std::invalid_argument If \p pFuture does not refer to a shared state.
 * \throws std::runtime_error If the operation timed out (\p pTimedOut set to true).
 * \throws boost::system::system_error If the handler throwed such an exception (other than from cancelling after timeout).
 *
 * \tparam ReturnT Return type of the handled operation (which is wrapped in \p pFuture).
 * \tparam T Type of the socket/port/resolver (%TCP or %UDP socket or resolver or a serial port from the Boost %ASIO library).
 * \param pFuture The future returned from initiating the async operation.
 * \param pSockPortResolver The socket, serial port or resolver on which the operation is performed.
 * \param pTimeout The timeout for the handled operation.
 * \param pTimedOut Whether \p pTimeout was reached (i.e. \p pSockPortResolver cancelled and thrown exception was because of the timeout).
 * \return Result of \p pFuture / the operation.
 */
template<typename ReturnT, typename T>
    requires (IsCancellableSocketOrSerPort<T> || IsCancellableResolver<T>)
ReturnT getAsyncBoostFutureWithTimedOutCancel(std::future<ReturnT>& pFuture, T& pSockPortResolver, std::chrono::milliseconds pTimeout,
                                              const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pTimedOut.has_value())
        pTimedOut->get() = false;

    if (!pFuture.valid())
        throw std::invalid_argument("Invalid future.");

    const std::future_status status = pFuture.wait_for(pTimeout);

    if (status == std::future_status::ready)
    {
        return pFuture.get();   //This may throw an exception
    }
    else if (status == std::future_status::timeout)
    {
        pSockPortResolver.cancel();

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
 * \brief Wait for the promised future, get and return its value; cancel the socket/port on timeout but return future's value anyway.
 *
 * This function should be used to retrieve the number of transferred bytes of an asynchronous operation on a socket/port \p pSocketOrPort
 * using readWriteHandler() as handler function when the bytes already transferred after a timeout must be processed in any case.
 * After \p pTimeout with still unfinished operation \p pSocketOrPort will be cancelled. In this case the expected
 * handler readWriteHandler() ensures that the number of already transferred bytes can still be obtained without
 * an exception being thrown (in contrast to the built-in \c boost::asio::use_future handler). As soon as the
 * handler completes by itself or after \p pTimeout the number of transferred bytes is returned.
 *
 * Note that \p pTimedOut is always set to false in the beginning, if defined, and set to true when the timeout happens.
 *
 * \throws std::invalid_argument If \p pPromiseN has no shared state or already stores a value/exception.
 *
 * \tparam T Type of the socket/port (%TCP or %UDP socket or serial port from the Boost %ASIO library).
 * \param pPromiseN The transferred bytes promise from the handler readWriteHandler().
 * \param pSocketOrPort The socket or serial port on which the operation is performed.
 * \param pTimeout The timeout for the handled operation.
 * \param pTimedOut Whether \p pTimeout was reached (i.e. \p pSocketOrPort cancelled and transferred bytes maybe less than expected).
 * \return Number of successfully transferred bytes.
 */
template<typename T>
    requires IsCancellableSocketOrSerPort<T>
std::size_t getAsyncTransferredWithTimedOutCancel(std::promise<std::size_t>& pPromiseN, T& pSocketOrPort, std::chrono::milliseconds pTimeout,
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
        pSocketOrPort.cancel();

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
/// \endcond INTERNAL

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_COMMONIMPL_ASIOHELPER_ASIOHELPER_H
