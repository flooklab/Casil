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

#include <casil/TL/CommonImpl/udpsocketwrapper.h>

#include <casil/asio.h>
#include <casil/TL/CommonImpl/asiohelper.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/system/system_error.hpp>

#include <future>
#include <stdexcept>
#include <utility>

using casil::Layers::TL::CommonImpl::UDPSocketWrapper;

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pHostName
 * \param pPort
 */
UDPSocketWrapper::UDPSocketWrapper(std::string pHostName, const int pPort) :
    hostName(std::move(pHostName)),
    port(pPort),
    socket(ASIO::getIOContext()),
    readBuffer()
{
}

//Public

/*!
 * \brief Receive a single datagram from the socket.
 *
 * \todo Detailed doc
 *
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
std::vector<std::uint8_t> UDPSocketWrapper::read(const std::chrono::milliseconds pTimeout,
                                                 const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    std::size_t n = 0;

    try
    {
        if (pTimeout <= std::chrono::milliseconds::zero())
            n = socket.receive(boost::asio::buffer(readBuffer, readBufferSize));
        else
        {
            std::promise<std::size_t> promiseN;

            socket.async_receive(boost::asio::buffer(readBuffer, readBufferSize),
                                 std::bind(&ASIOHelper::readWriteHandler, std::placeholders::_1, std::placeholders::_2, std::ref(promiseN)));

            n = ASIOHelper::getAsyncTransferredWithTimedOutCancel(promiseN, socket, pTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while reading from UDP socket: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Unexpected runtime error (THIS SHOULD NEVER HAPPEN!): ") + exc.what());
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Invalid promise argument. THIS SHOULD NEVER HAPPEN!");
    }

    return std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + n);
}

/*!
 * \brief Receive maximally some amount of bytes of a single datagram from the socket.
 *
 * \todo Detailed doc
 *
 * \param pSize
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
std::vector<std::uint8_t> UDPSocketWrapper::readMax(const int pSize, const std::chrono::milliseconds pTimeout,
                                                    const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pSize > 0)
    {
        std::size_t n = 0;

        try
        {
            if (pTimeout <= std::chrono::milliseconds::zero())
                n = socket.receive(boost::asio::buffer(readBuffer, static_cast<std::size_t>(pSize)));
            else
            {
                std::promise<std::size_t> promiseN;

                socket.async_receive(boost::asio::buffer(readBuffer, static_cast<std::size_t>(pSize)),
                                     std::bind(&ASIOHelper::readWriteHandler,
                                               std::placeholders::_1, std::placeholders::_2, std::ref(promiseN)));

                n = ASIOHelper::getAsyncTransferredWithTimedOutCancel(promiseN, socket, pTimeout, pTimedOut);
            }
        }
        catch (const boost::system::system_error& exc)
        {
            throw std::runtime_error(std::string("Exception while reading from UDP socket: ") + exc.what());
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error(std::string("Unexpected runtime error (THIS SHOULD NEVER HAPPEN!): ") + exc.what());
        }
        catch (const std::invalid_argument&)
        {
            throw std::runtime_error("Invalid promise argument. THIS SHOULD NEVER HAPPEN!");
        }

        return std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + n);
    }
    else
        return {};
}

/*!
 * \brief Send a single datagram over the socket.
 *
 * \todo Detailed doc
 *
 * \param pData
 * \param pTimeout
 * \param pTimedOut
 */
void UDPSocketWrapper::write(const std::vector<std::uint8_t>& pData, const std::chrono::milliseconds pTimeout,
                             const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    try
    {
        if (pTimeout <= std::chrono::milliseconds::zero())
            socket.send(boost::asio::buffer(pData.data(), pData.size()));
        else
        {
            std::future<std::size_t> futureN = socket.async_send(boost::asio::buffer(pData.data(), pData.size()), boost::asio::use_future);

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(futureN, socket, pTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to UDP socket: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to UDP socket: ") + exc.what());
    }
}

//

/*!
 * \brief Check if no incoming datagrams are available on the socket.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool UDPSocketWrapper::readBufferEmpty() const
{
    try
    {
        return (socket.available() == 0);
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while getting read buffer size from UDP socket: ") + exc.what());
    }
}

/*!
 * \brief Read remaining datagrams from the socket and discard them.
 *
 * \todo Detailed doc
 */
void UDPSocketWrapper::clearReadBuffer()
{
    try
    {
        while (!readBufferEmpty())
            socket.receive(boost::asio::buffer(readBuffer, readBufferSize));
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while reading from UDP socket: ") + exc.what());
    }
    catch (const std::runtime_error&)
    {
        throw;
    }
}

//

/*!
 * \brief Connect the %UDP socket.
 *
 * \todo Detailed doc
 *
 * \param pConnectTimeout
 * \param pTimedOut
 */
void UDPSocketWrapper::init(const std::chrono::milliseconds pConnectTimeout, const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (!ASIO::ioContextThreadsRunning())
        throw std::runtime_error("Using a UDP socket requires running at least one IO context thread.");

    try
    {
        boost::asio::ip::udp::resolver resolver(ASIO::getIOContext());
        boost::asio::ip::udp::resolver::query udpQuery(hostName, std::to_string(port));

        if (pConnectTimeout <= std::chrono::milliseconds::zero())
            boost::asio::connect(socket, resolver.resolve(udpQuery));
        else
        {
            std::future<boost::asio::ip::udp::endpoint> endpoint = boost::asio::async_connect(socket,
                                                                                              resolver.resolve(udpQuery),
                                                                                              boost::asio::use_future);

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(endpoint, socket, pConnectTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while connecting UDP socket: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Exception while connecting UDP socket: ") + exc.what());
    }
}

/*!
 * \brief Disconnect the %UDP socket.
 *
 * \todo Detailed doc
 */
void UDPSocketWrapper::close()
{
    try
    {
        socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
        socket.close();
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while closing UDP socket: ") + exc.what());
    }
}
