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

#include <casil/TL/CommonImpl/tcpsocketwrapper.h>

#include <casil/asio.h>
#include <casil/bytes.h>
#include <casil/TL/CommonImpl/asiohelper.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/system_error.hpp>

#include <future>
#include <algorithm>
#include <stdexcept>
#include <utility>

using casil::TL::CommonImpl::TCPSocketWrapper;

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pHostName
 * \param pPort
 * \param pReadTermination
 * \param pWriteTermination
 */
TCPSocketWrapper::TCPSocketWrapper(std::string pHostName, const int pPort,
                                   std::string pReadTermination, const std::string& pWriteTermination) :
    hostName(std::move(pHostName)),
    port(pPort),
    readTermination(std::move(pReadTermination)),
    readTerminationLength(readTermination.length()),
    writeTermination(Bytes::byteVecFromStr(pWriteTermination)),
    writeTerminationLength(writeTermination.size()),
    socket(ASIO::getIOContext()),
    readBuffer()
{
}

//Public

/*!
 * \brief Read an amount of bytes from the socket, or until read termination.
 *
 * \todo Detailed doc
 *
 * \param pSize
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
std::vector<std::uint8_t> TCPSocketWrapper::read(const int pSize, const std::chrono::milliseconds pTimeout,
                                                 const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pSize == -1)
    {
        std::size_t n = 0;

        try
        {
            if (pTimeout <= std::chrono::milliseconds::zero())
                n = boost::asio::read_until(socket, boost::asio::dynamic_buffer(readBuffer), readTermination);
            else
            {
                std::promise<std::size_t> promiseN;

                boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(readBuffer), readTermination,
                                              std::bind(&ASIOHelper::readWriteHandler,
                                                        std::placeholders::_1, std::placeholders::_2, std::ref(promiseN)));

                n = ASIOHelper::getAsyncTransferredWithTimedOutCancel(promiseN, socket, pTimeout, pTimedOut);
            }
        }
        catch (const boost::system::system_error& exc)
        {
            //Workaround: always close on EOF; succeeding async read calls would apparently not fail again otherwise
            if (exc.code().value() == boost::system::errc::no_such_file_or_directory && socket.is_open())
            {
                try
                {
                    close();
                }
                catch (const std::runtime_error&)
                {
                }
            }

            throw std::runtime_error(std::string("Exception while reading from TCP socket: ") + exc.what());
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error(std::string("Unexpected runtime error (THIS SHOULD NEVER HAPPEN!): ") + exc.what());
        }
        catch (const std::invalid_argument&)
        {
            throw std::runtime_error("Invalid promise argument. THIS SHOULD NEVER HAPPEN!");
        }

        if (n < readTerminationLength)  //According to Boost documentation this should not even happen; check anyway because of timeout
        {
            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + n);

            throw std::runtime_error("Error while reading from TCP socket: Did not read until read termination.");
        }

        std::vector<std::uint8_t> retVal(readBuffer.begin(), readBuffer.begin() + n - readTerminationLength);

        readBuffer.erase(readBuffer.begin(), readBuffer.begin() + n);

        return retVal;
    }
    else if (pSize > 0)
    {
        if (std::cmp_less(readBuffer.size(), pSize))
        {
            const std::size_t oldSize = readBuffer.size();

            readBuffer.resize(pSize);

            try
            {
                try
                {
                    if (pTimeout <= std::chrono::milliseconds::zero())
                        boost::asio::read(socket, boost::asio::buffer(readBuffer.data() + oldSize, pSize - oldSize));
                    else
                    {
                        std::promise<std::size_t> promiseN;

                        boost::asio::async_read(socket, boost::asio::buffer(readBuffer.data() + oldSize, pSize - oldSize),
                                                std::bind(&ASIOHelper::readWriteHandler,
                                                          std::placeholders::_1, std::placeholders::_2, std::ref(promiseN)));

                        (void)ASIOHelper::getAsyncTransferredWithTimedOutCancel(promiseN, socket, pTimeout, pTimedOut);
                    }
                }
                catch (const boost::system::system_error& exc)
                {
                    //Workaround: always close on EOF; succeeding async read calls would apparently not fail again otherwise
                    if (exc.code().value() == boost::system::errc::no_such_file_or_directory && socket.is_open())
                    {
                        try
                        {
                            close();
                        }
                        catch (const std::runtime_error&)
                        {
                        }
                    }

                    throw std::runtime_error(std::string("Exception while reading from TCP socket: ") + exc.what());
                }
                catch (const std::runtime_error& exc)
                {
                    throw std::runtime_error(std::string("Unexpected runtime error (THIS SHOULD NEVER HAPPEN!): ") + exc.what());
                }
                catch (const std::invalid_argument&)
                {
                    throw std::runtime_error("Invalid promise argument. THIS SHOULD NEVER HAPPEN!");
                }
            }
            catch (const std::runtime_error&)
            {
                readBuffer.resize(oldSize);     //Need to restore previous size
                throw;
            }
        }

        if (std::cmp_equal(readBuffer.size(), pSize))
        {
            std::vector<std::uint8_t> retVal;

            retVal.swap(readBuffer);

            return retVal;
        }
        else
        {
            std::vector<std::uint8_t> retVal = std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + pSize);

            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + pSize);

            return retVal;
        }
    }
    else
        return {};
}

/*!
 * \brief Read maximally some amount of bytes from the socket.
 *
 * \todo Detailed doc
 *
 * \param pSize
 * \param pTimeout
 * \param pTimedOut
 * \return
 */
std::vector<std::uint8_t> TCPSocketWrapper::readMax(const int pSize, const std::chrono::milliseconds pTimeout,
                                                    const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pSize > 0)
    {
        std::size_t n = 0;

        if (readBuffer.size() == 0)
        {
            readBuffer.resize(pSize);

            try
            {
                try
                {
                    if (pTimeout <= std::chrono::milliseconds::zero())
                        n = boost::asio::read(socket, boost::asio::buffer(readBuffer.data(), pSize), boost::asio::transfer_at_least(1));
                    else
                    {
                        std::promise<std::size_t> promiseN;

                        boost::asio::async_read(socket, boost::asio::buffer(readBuffer.data(), pSize), boost::asio::transfer_at_least(1),
                                                std::bind(&ASIOHelper::readWriteHandler,
                                                          std::placeholders::_1, std::placeholders::_2, std::ref(promiseN)));

                        n = ASIOHelper::getAsyncTransferredWithTimedOutCancel(promiseN, socket, pTimeout, pTimedOut);
                    }
                }
                catch (const boost::system::system_error& exc)
                {
                    //Workaround: always close on EOF; succeeding async read calls would apparently not fail again otherwise
                    if (exc.code().value() == boost::system::errc::no_such_file_or_directory && socket.is_open())
                    {
                        try
                        {
                            close();
                        }
                        catch (const std::runtime_error&)
                        {
                        }
                    }

                    throw std::runtime_error(std::string("Exception while reading from TCP socket: ") + exc.what());
                }
                catch (const std::runtime_error& exc)
                {
                    throw std::runtime_error(std::string("Unexpected runtime error (THIS SHOULD NEVER HAPPEN!): ") + exc.what());
                }
                catch (const std::invalid_argument&)
                {
                    throw std::runtime_error("Invalid promise argument. THIS SHOULD NEVER HAPPEN!");
                }
            }
            catch (const std::runtime_error&)
            {
                readBuffer.clear();     //Need to restore zero size
                throw;
            }
        }
        else
            n = readBuffer.size();

        const std::size_t readNum = std::min(n, static_cast<std::size_t>(pSize));

        if (std::cmp_equal(readNum, readBuffer.size()))
        {
            std::vector<std::uint8_t> retVal;

            retVal.swap(readBuffer);

            return retVal;
        }
        else
        {
            std::vector<std::uint8_t> retVal(readBuffer.begin(), readBuffer.begin() + readNum);

            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + readNum);

            std::size_t remainingBufferSize = n - readNum;

            readBuffer.resize(remainingBufferSize);

            return retVal;
        }
    }
    else
        return {};
}

/*!
 * \brief Write data to the socket (automatically terminated).
 *
 * \todo Detailed doc
 *
 * \param pData
 * \param pTimeout
 * \param pTimedOut
 */
void TCPSocketWrapper::write(const std::vector<std::uint8_t>& pData, const std::chrono::milliseconds pTimeout,
                             const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    try
    {
        if (pTimeout <= std::chrono::milliseconds::zero())
        {
            boost::asio::write(socket, boost::asio::buffer(pData.data(), pData.size()));
            boost::asio::write(socket, boost::asio::buffer(writeTermination));
        }
        else
        {
            std::future<std::size_t> futureN = boost::asio::async_write(socket, boost::asio::buffer(pData.data(), pData.size()),
                                                                        boost::asio::use_future);

            const auto timeoutRefTime = std::chrono::steady_clock::now();

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(futureN, socket, pTimeout, pTimedOut);

            futureN = boost::asio::async_write(socket, boost::asio::buffer(writeTermination), boost::asio::use_future);

            const auto reducedTimeout = pTimeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                                                         timeoutRefTime);

            if (reducedTimeout <= std::chrono::milliseconds::zero())
            {
                if (pTimedOut.has_value())
                    pTimedOut->get() = true;
                throw std::runtime_error("Timeout.");
            }

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(futureN, socket, reducedTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to TCP socket: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to TCP socket: ") + exc.what());
    }
}

//

/*!
 * \brief Check if the read buffer is empty (and no remaining data to be read).
 *
 * \todo Detailed doc
 *
 * \return
 */
bool TCPSocketWrapper::readBufferEmpty() const
{
    if (readBuffer.size() > 0)
        return false;

    try
    {
        return (socket.available() == 0);
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while getting read buffer size from TCP socket: ") + exc.what());
    }

    return false;
}

/*!
 * \brief Read remaining data from the socket and then clear the read buffer contents.
 *
 * \todo Detailed doc
 */
void TCPSocketWrapper::clearReadBuffer()
{
    std::size_t dataAvail = 0;

    try
    {
        dataAvail = socket.available();
    }
    catch (const boost::system::system_error& exc)
    {
        readBuffer.clear();
        throw std::runtime_error(std::string("Exception while getting read buffer size from TCP socket: ") + exc.what());
    }

    if (readBuffer.size() < dataAvail)
        readBuffer.resize(dataAvail);

    try
    {
        boost::asio::read(socket, boost::asio::buffer(readBuffer.data(), dataAvail));
    }
    catch (const boost::system::system_error& exc)
    {
        readBuffer.clear();
        throw std::runtime_error(std::string("Exception while reading from TCP socket: ") + exc.what());
    }

    readBuffer.clear();
}

//

/*!
 * \brief Connect the %TCP socket.
 *
 * \todo Detailed doc
 *
 * \param pConnectTimeout
 * \param pTimedOut
 */
void TCPSocketWrapper::init(const std::chrono::milliseconds pConnectTimeout, const std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (!ASIO::ioContextThreadsRunning())
        throw std::runtime_error("Using a TCP socket requires running at least one IO context thread.");

    try
    {
        boost::asio::ip::tcp::resolver resolver(ASIO::getIOContext());
        boost::asio::ip::tcp::resolver::query tcpQuery(hostName, std::to_string(port));

        if (pConnectTimeout <= std::chrono::milliseconds::zero())
            boost::asio::connect(socket, resolver.resolve(tcpQuery));
        else
        {
            std::future<boost::asio::ip::tcp::endpoint> endpoint = boost::asio::async_connect(socket,
                                                                                              resolver.resolve(tcpQuery),
                                                                                              boost::asio::use_future);

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(endpoint, socket, pConnectTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while connecting TCP socket: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Exception while connecting TCP socket: ") + exc.what());
    }
}

/*!
 * \brief Disconnect the %TCP socket.
 *
 * \todo Detailed doc
 */
void TCPSocketWrapper::close()
{
    try
    {
        try
        {
            socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        }
        catch (const boost::system::system_error& exc)
        {
            if (exc.code().value() != boost::system::errc::not_connected)   //No need to propagate this exception if just not connected
                throw;
        }

        socket.close();
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while closing TCP socket: ") + exc.what());
    }
}
