/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/system/system_error.hpp>

#include <future>
#include <stdexcept>
#include <utility>

/// \cond INTERNAL

using casil::Layers::TL::CommonImpl::UDPSocketWrapper;

//

/*!
 * \brief Constructor.
 *
 * Note: Initializes the socket using the IO context from ASIO::getIOContext().
 *
 * \param pHostName Host name of the remote endpoint.
 * \param pPort Network port to be used.
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
 * Reads one datagram from the socket and returns the contained data as byte sequence.
 *
 * If \p pTimeout is non-zero and that timeout is reached, \p pTimedOut will
 * be set to true (if defined) and the already read bytes will be returned.
 *
 * \throws std::runtime_error If reading from the socket fails.
 *
 * \param pTimeout The timeout for the read operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 * \return Payload byte sequence of requested datagram.
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
 * Reads one datagram from the socket, stopping after maximally \p pSize bytes, and returns the contained data as byte sequence.
 * Returns an empty sequence for negative \p pSize.
 *
 * If \p pTimeout is non-zero and that timeout is reached, \p pTimedOut will
 * be set to true (if defined) and the already read bytes will be returned.
 *
 * \throws std::runtime_error If reading from the socket fails.
 *
 * \param pSize Maximum number of bytes to read from the datagram.
 * \param pTimeout The timeout for the read operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 * \return Maximally \p pSize bytes of the payload of the requested datagram.
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
 * Writes a datagram containing \p pData to the socket.
 * If \p pTimeout is non-zero, it is used as timeout for the write attempt.
 * If the timeout is reached, \p pTimedOut will be set to true (if defined) and an exception is thrown.
 *
 * \throws std::runtime_error On timeout.
 * \throws std::runtime_error If writing to the socket fails.
 *
 * \param pData Data to be written.
 * \param pTimeout The timeout for the write operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
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
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Invalid future argument. THIS SHOULD NEVER HAPPEN!");
    }
}

//

/*!
 * \brief Check if no incoming datagrams are available on the socket.
 *
 * \throws std::runtime_error If accessing the socket fails.
 *
 * \return True if there are no datagrams available.
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
 * Iteratively reads and discards available datagrams until readBufferEmpty() returns true.
 *
 * \throws std::runtime_error If accessing the socket fails.
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
 * Resolves the configured host name and connects the socket to this endpoint via the configured port.
 * If \p pConnectTimeout is non-zero, it is used as timeout for the connection attempt.
 * If the timeout is reached, \p pTimedOut will be set to true (if defined) and an exception is thrown.
 *
 * \throws std::runtime_error If no IO context threads are running (see ASIO::ioContextThreadsRunning()).
 * \throws std::runtime_error On timeout.
 * \throws std::runtime_error If resolving the host name or connecting the socket fails.
 *
 * \param pConnectTimeout The timeout for the connection attempt.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 */
void UDPSocketWrapper::init(const std::chrono::milliseconds pConnectTimeout, const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (!ASIO::ioContextThreadsRunning())
        throw std::runtime_error("Using a UDP socket requires running at least one IO context thread.");

    try
    {
        boost::asio::ip::udp::resolver resolver(ASIO::getIOContext());

        if (pConnectTimeout <= std::chrono::milliseconds::zero())
            boost::asio::connect(socket, resolver.resolve(hostName, std::to_string(port)));
        else
        {
            std::future<boost::asio::ip::udp::endpoint> endpoint = boost::asio::async_connect(socket,
                                                                                              resolver.resolve(hostName, std::to_string(port)),
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
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Invalid future argument. THIS SHOULD NEVER HAPPEN!");
    }
}

/*!
 * \brief Disconnect the %UDP socket.
 *
 * Shuts down send/receive operations and closes the socket.
 *
 * \throws std::runtime_error If shutting down or closing the socket fails.
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

/// \endcond INTERNAL
