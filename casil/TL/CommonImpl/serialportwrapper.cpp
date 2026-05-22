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

#include <casil/TL/CommonImpl/serialportwrapper.h>

#include <casil/asio.h>
#include <casil/bytes.h>
#include <casil/logger.h>
#include <casil/TL/CommonImpl/asiohelper.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <future>
#include <stdexcept>
#include <utility>

/// \cond INTERNAL

using casil::Layers::TL::CommonImpl::SerialPortWrapper;

//

/*!
 * \brief Constructor.
 *
 * Note: Initializes the serial port using the IO context from ASIO::getIOContext().
 *
 * \param pPort %Device name of the serial port to be used.
 * \param pReadTermination Termination sequence for non-sized read operations.
 * \param pWriteTermination Termination sequence to append for write operations.
 * \param pBaudRate Baud rate to be used for the serial connection.
 * \param pCharacterSize Number of character bits (use 5, 6, 7 or 8) to be used for the serial connection.
 * \param pParity Parity setting for the serial connection.
 * \param pStopBits Stop bit setting for the serial connection.
 * \param pFlowControl Flow control setting for the serial connection.
 */
SerialPortWrapper::SerialPortWrapper(std::string pPort, const std::string& pReadTermination, const std::string& pWriteTermination,
                                     const unsigned int pBaudRate, const unsigned int pCharacterSize,
                                     const PortParity pParity, const PortStopBits pStopBits, const PortFlowControl pFlowControl) :
    port(std::move(pPort)),
    readTermination(Bytes::byteVecFromStr(pReadTermination)),
    readTerminationLength(readTermination.size()),
    writeTermination(Bytes::byteVecFromStr(pWriteTermination)),
    writeTerminationLength(writeTermination.size()),
    baudRate(pBaudRate),
    characterSize(pCharacterSize),
    parityOption(pParity),
    stopBitsOption(pStopBits),
    flowControlOption(pFlowControl),
    serialPort(ASIO::getIOContext()),
    readBuffer(),
    intermediateReadBuffer(),
    readBufferMutex(),
    pollData(false),
    pollDataStopped(false),
    newData(false),
    newDataCondVar(),
    bufferErrorCount(0)
{
}

/*!
 * \brief Destructor.
 *
 * Calls close() if read data polling (see init()) was not stopped yet (i.e. close() not called yet).
 *
 * Note: Returns silently even if close() fails by throwing \c std::runtime_error.
 */
SerialPortWrapper::~SerialPortWrapper()
{
    if (pollData.load())    //Not closed yet; need to stop polling here
    {
        try
        {
            close();
        }
        catch (const std::runtime_error&)
        {
        }
    }
}

//Public

/*!
 * \brief Read an amount of bytes from the read buffer, or until read termination.
 *
 * Reads and returns \p pSize bytes from the read buffer if \p pSize is positive and any
 * number of bytes up to (but excluding) the configured read termination if \p pSize is -1.
 * Other values for \p pSize are not useful (will then return an empty sequence).
 *
 * If \p pTimeout is non-zero and that timeout is reached, \p pTimedOut will be set to true (if defined) and
 * the \e already read bytes will be returned, filled with trailing zeros to \p pSize for positive \p pSize.
 * Note that, if reading until termination (\p pSize equal -1) in such a case, the read termination
 * will \e only be excluded from the returned sequence if it was \e fully read into the buffer
 * (which is unlikely but can actually happen). Otherwise \e nothing will be stripped off.
 *
 * The same applies to \p pInterCharTimeout, except that this is not a total timeout for the whole fuction call but is rather
 * used to limit the time between arriving partial chunks of data (which could also be just individual bytes/characters,
 * hence the name). This means that every time that any amount of new data arrives from the ASIO library that timeout
 * is reset and starts again from zero. Note that both timeouts can be used simultaneously.
 *
 * \note When the read buffer polling stops (or if it stopped already) because of repeated read errors (see handleAsyncRead())
 *       \e before the requested data gets complete (i.e. still waiting for termination or \p pSize), then this
 *       function will \e not block indefinitely but instead return the available incomplete data regardless.
 *       In case of positive \p pSize, the returned data will be filled with trailing zeros to \p pSize.
 *
 * \note \e If the returned data is incomplete/filled (due to timeout or polling stopped), this will be logged as an error.
 *
 * \param pSize Number of bytes to read or -1.
 * \param pTimeout The overall/maximum timeout for the read operation.
 * \param pInterCharTimeout The "inter-character" timeout for the read operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 * \return Byte sequence of requested length or up to (but excluding) termination.
 */
std::vector<std::uint8_t> SerialPortWrapper::read(const int pSize, const std::chrono::milliseconds pTimeout,
                                                  const std::chrono::milliseconds pInterCharTimeout,
                                                  std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    std::unique_lock<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    //Assign optional reference if was not passed by caller in order to facilitate communication of timeout within this function
    bool tTimedOutFallback = false;                                     // cppcheck-suppress variableScope symbolName=tTimedOutFallback
    if (!pTimedOut.has_value())
        pTimedOut = std::ref(tTimedOutFallback);

    const bool useMaxTimeout = (pTimeout > std::chrono::milliseconds::zero());
    const bool useInterTimeout = (pInterCharTimeout > std::chrono::milliseconds::zero());
    const auto tMax = std::chrono::steady_clock::now() + pTimeout;

    auto waitForNewData = [this, &bufferLock, useMaxTimeout, useInterTimeout, tMax, pInterCharTimeout, &pTimedOut]() -> bool
    {
        if (pollDataStopped.load())
            return false;

        newData = false;

        if (useMaxTimeout || useInterTimeout)
        {
            auto tNext = tMax;

            if (useInterTimeout)
            {
                const auto tInter = std::chrono::steady_clock::now() + pInterCharTimeout;
                if (!useMaxTimeout || (useMaxTimeout && (tInter < tMax)))
                    tNext = tInter;
            }

            if (!newDataCondVar.wait_until(bufferLock, tNext, [this](){ return newData; }))
            {
                pTimedOut->get() = true;
                Logger::logWarning("Timeout was reached while trying to read from serial port \"" + port + "\".");
                return false;
            }
        }
        else
            newDataCondVar.wait(bufferLock, [this](){ return newData; });

        return !pollDataStopped.load();
    };

    if (pSize == -1)
    {
        auto termPos = std::search(readBuffer.begin(), readBuffer.end(), readTermination.begin(), readTermination.end());

        while (termPos == readBuffer.end())
        {
            const bool ok = waitForNewData();
            termPos = std::search(readBuffer.begin(), readBuffer.end(), readTermination.begin(), readTermination.end());
            if (!ok)
            {
                if (termPos == readBuffer.end())
                {
                    if (pTimedOut->get() == true)
                    {
                        Logger::logError("Problem while reading from serial port \"" + port + "\": Could not read "
                                         "up to termination because the timeout was reached.");
                    }
                    else
                    {
                        Logger::logError("Problem while reading from serial port \"" + port + "\": Could not read "
                                         "up to termination because polling was stopped due to previous errors.");
                    }
                }
                break;
            }
        }

        std::vector<std::uint8_t> retVal(readBuffer.begin(), termPos);

        if (termPos == readBuffer.end())    //Only possible if above waiting returned with error (timeout or polling stopped)
            readBuffer.clear();
        else
            readBuffer.erase(readBuffer.begin(), termPos + readTerminationLength);

        return retVal;
    }
    else if (pSize > 0)
    {
        while (std::cmp_less(readBuffer.size(), pSize))
        {
            if (waitForNewData())
                continue;
            else
            {
                //Either timeout or polling stopped from errors, hence need to handle possibly incomplete data
                if (std::cmp_less(readBuffer.size(), pSize))
                {
                    if (pTimedOut->get() == true)
                    {
                        Logger::logError("Problem while reading from serial port \"" + port + "\": Could not read requested number "
                                         "of bytes because the timeout was reached. Filling with zeros...");
                    }
                    else
                    {
                        Logger::logError("Problem while reading from serial port \"" + port + "\": Could not read requested number "
                                         "of bytes because polling was stopped due to previous errors. Filling with zeros...");
                    }

                    //Fill returned data with zeros to obtain requested data size
                    std::vector<std::uint8_t> retVal(readBuffer.begin(), readBuffer.begin() + readBuffer.size());
                    retVal.resize(pSize, 0);

                    readBuffer.clear();

                    return retVal;
                }
                else
                    break;  //If read data size is sufficient despite error, continue with regular processing
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
            std::vector<std::uint8_t> retVal(readBuffer.begin(), readBuffer.begin() + pSize);

            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + pSize);

            return retVal;
        }
    }
    else
        return {};
}

/*!
 * \brief Read maximally some amount of bytes from the read buffer.
 *
 * Reads and returns maximally \p pSize bytes from the read buffer. Returns an empty sequence for negative \p pSize.
 *
 * If \p pTimeout is non-zero and that timeout is reached before any bytes were read, \p pTimedOut will be set to true (if defined)
 * and the function simply returns the current buffer content, which should be an \e empty byte sequence in this case.
 *
 * \note This function normally waits until at least \e some data is available. However, when the read buffer polling
 *       stops (or if it stopped already) because of repeated read errors (see handleAsyncRead()) \e before any data
 *       got available, then this function will \e not block indefinitely but instead just return an empty byte sequence.
 *
 * \note \e If the returned data is empty (due to timeout or polling stopped), this will be logged as a warning.
 *
 * \param pSize Maximum number of bytes to read.
 * \param pTimeout The timeout for the read operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 * \return Maximally \p pSize bytes long byte sequence.
 */
std::vector<std::uint8_t> SerialPortWrapper::readMax(const int pSize, const std::chrono::milliseconds pTimeout,
                                                     std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    if (pSize > 0)
    {
        std::unique_lock<std::mutex> bufferLock(readBufferMutex);
        (void)bufferLock;

        //Assign optional reference if was not passed by caller in order to facilitate communication of timeout within this function
        bool tTimedOutFallback = false;                                     // cppcheck-suppress variableScope symbolName=tTimedOutFallback
        if (!pTimedOut.has_value())
            pTimedOut = std::ref(tTimedOutFallback);

        auto waitForNewData = [this, &bufferLock, pTimeout, &pTimedOut]() -> bool
        {
            if (pollDataStopped.load())
                return false;

            newData = false;

            if (pTimeout > std::chrono::milliseconds::zero())
            {
                if (!newDataCondVar.wait_until(bufferLock, std::chrono::steady_clock::now() + pTimeout, [this](){ return newData; }))
                {
                    pTimedOut->get() = true;
                    Logger::logWarning("Timeout was reached while trying to read from serial port \"" + port + "\".");
                    return false;
                }
            }
            else
                newDataCondVar.wait(bufferLock, [this](){ return newData; });

            return !pollDataStopped.load();
        };

        if (readBuffer.size() == 0)
        {
            if (!waitForNewData())
            {
                if (pTimedOut->get() == true)
                {
                    Logger::logWarning("Problem while reading from serial port \"" + port + "\": "
                                       "Could not read any data because the timeout was reached.");
                }
                else
                {
                    Logger::logWarning("Problem while reading from serial port \"" + port + "\": Could not properly "
                                       "read the data because polling was stopped due to previous errors.");
                }
            }
        }

        std::size_t readNum = std::min(readBuffer.size(), static_cast<std::size_t>(pSize));

        if (readNum == readBuffer.size())
        {
            std::vector<std::uint8_t> retVal;

            retVal.swap(readBuffer);

            return retVal;
        }
        else
        {
            std::vector<std::uint8_t> retVal(readBuffer.begin(), readBuffer.begin() + readNum);

            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + readNum);

            return retVal;
        }
    }
    else
        return {};
}

/*!
 * \brief Write data to the port (automatically terminated).
 *
 * Writes \p pData to the serial port, automatically followed by the configured write termination.
 * If \p pTimeout is non-zero, it is used as timeout for the write attempt.
 * If the timeout is reached, \p pTimedOut will be set to true (if defined) and an exception is thrown.
 *
 * \throws std::runtime_error On timeout.
 * \throws std::runtime_error If accessing the serial port fails.
 *
 * \param pData Data to be written (excluding termination).
 * \param pTimeout The timeout for the write operation.
 * \param pTimedOut Gets set (if defined) when \p pTimeout was reached.
 */
void SerialPortWrapper::write(const std::vector<std::uint8_t>& pData, const std::chrono::milliseconds pTimeout,
                              std::optional<std::reference_wrapper<bool>> pTimedOut)
{
    try
    {
        if (pTimeout <= std::chrono::milliseconds::zero())
        {
            boost::asio::write(serialPort, boost::asio::buffer(pData.data(), pData.size()));
            boost::asio::write(serialPort, boost::asio::buffer(writeTermination));
        }
        else
        {
            std::future<std::size_t> futureN = boost::asio::async_write(serialPort, boost::asio::buffer(pData.data(), pData.size()),
                                                                        boost::asio::use_future);

            const auto timeoutRefTime = std::chrono::steady_clock::now();

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(futureN, serialPort, pTimeout, pTimedOut);

            futureN = boost::asio::async_write(serialPort, boost::asio::buffer(writeTermination), boost::asio::use_future);

            const auto reducedTimeout = pTimeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                                                         timeoutRefTime);

            if (reducedTimeout <= std::chrono::milliseconds::zero())
            {
                if (pTimedOut.has_value())
                    pTimedOut->get() = true;
                throw std::runtime_error("Timeout.");
            }

            (void)ASIOHelper::getAsyncBoostFutureWithTimedOutCancel(futureN, serialPort, reducedTimeout, pTimedOut);
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to serial port: ") + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to serial port: ") + exc.what());
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Invalid future argument. THIS SHOULD NEVER HAPPEN!");
    }
}

//

/*!
 * \brief Check if the read buffer is empty.
 *
 * \return True if no data is available to be read.
 */
bool SerialPortWrapper::readBufferEmpty() const
{
    const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    return readBuffer.empty();
}

/*!
 * \brief Clear the current contents of the read buffer.
 */
void SerialPortWrapper::clearReadBuffer()
{
    const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    readBuffer.clear();
}

//

/*!
 * \brief Open the serial port and start continuous read buffer polling.
 *
 * Opens the serial port using the configured device name, sets the configured baud rate and other options (character size,
 * parity, stop bits, flow control) and enables continuous read buffer polling and starts it by calling pollReadBuffer().
 *
 * \throws std::runtime_error If no IO context threads are running (see ASIO::ioContextThreadsRunning()).
 * \throws std::runtime_error If opening the serial port or setting the baud rate or any of the other options fails.
 */
void SerialPortWrapper::init()
{
    if (!ASIO::ioContextThreadsRunning())
        throw std::runtime_error("Using a serial port requires running at least one IO context thread.");

    try
    {
        serialPort.open(port);
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while opening serial port: ") + exc.what());
    }

    try
    {
        serialPort.set_option(boost::asio::serial_port::baud_rate(baudRate));
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while setting baud rate for serial port: ") + exc.what());
    }

    try
    {
        serialPort.set_option(boost::asio::serial_port::character_size(characterSize));
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while setting character size for serial port: ") + exc.what());
    }

    try
    {
        using parity = boost::asio::serial_port::parity;

        switch (parityOption)
        {
            case PortParity::None:
            {
                serialPort.set_option(parity(parity::none));
                break;
            }
            case PortParity::Odd:
            {
                serialPort.set_option(parity(parity::odd));
                break;
            }
            case PortParity::Even:
            {
                serialPort.set_option(parity(parity::even));
                break;
            }
            default:
                throw std::runtime_error("Exception while setting parity for serial port: Invalid parity option. THIS SHOULD NEVER HAPPEN!");
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while setting parity for serial port: ") + exc.what());
    }

    try
    {
        using stop_bits = boost::asio::serial_port::stop_bits;

        switch (stopBitsOption)
        {
            case PortStopBits::One:
            {
                serialPort.set_option(stop_bits(stop_bits::one));
                break;
            }
            case PortStopBits::OnePointFive:
            {
                serialPort.set_option(stop_bits(stop_bits::onepointfive));
                break;
            }
            case PortStopBits::Two:
            {
                serialPort.set_option(stop_bits(stop_bits::two));
                break;
            }
            default:
            {
                throw std::runtime_error("Exception while setting number of stop bits for serial port: "
                                         "Invalid stop bits option. THIS SHOULD NEVER HAPPEN!");
            }
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while setting number of stop bits for serial port: ") + exc.what());
    }

    try
    {
        using flow_control = boost::asio::serial_port::flow_control;

        switch (flowControlOption)
        {
            case PortFlowControl::None:
            {
                serialPort.set_option(flow_control(flow_control::none));
                break;
            }
            case PortFlowControl::Software:
            {
                serialPort.set_option(flow_control(flow_control::software));
                break;
            }
            case PortFlowControl::Hardware:
            {
                serialPort.set_option(flow_control(flow_control::hardware));
                break;
            }
            default:
            {
                throw std::runtime_error("Exception while setting flow control for serial port: "
                                         "Invalid flow control option. THIS SHOULD NEVER HAPPEN!");
            }
        }
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while setting flow control for serial port: ") + exc.what());
    }

    pollData.store(true);
    pollDataStopped.store(false);
    bufferErrorCount.store(0);

    pollReadBuffer();
}

/*!
 * \brief Stop the continuous read buffer polling and close the serial port.
 *
 * Disables continuous read buffer polling started by init() and waits until it has stopped.
 * Cancels pending asynchronous operations and closes the port.
 *
 * \throws std::runtime_error If cancelling or closing the serial port fails.
 */
void SerialPortWrapper::close()
{
    pollData.store(false);

    try
    {
        serialPort.cancel();

        pollDataStopped.wait(false);

        serialPort.close();
    }
    catch (const boost::system::system_error& exc)
    {
        pollDataStopped.wait(false);
        throw std::runtime_error(std::string("Exception while closing serial port: ") + exc.what());
    }
}

//Private

/*!
 * \brief Issue an async read to poll the serial port (handler is handleAsyncRead()).
 *
 * Starts an asynchronous operation to read at least one byte from the serial port
 * into an intermediate buffer and process those read bytes by handleAsyncRead().
 */
void SerialPortWrapper::pollReadBuffer()
{
    boost::asio::async_read(serialPort, boost::asio::dynamic_buffer(intermediateReadBuffer), boost::asio::transfer_at_least(1),
                            std::bind(&SerialPortWrapper::handleAsyncRead, this, std::placeholders::_1 ,std::placeholders::_2));
}

/*!
 * \brief Fill read buffer from single poll by pollReadBuffer() and issue next poll.
 *
 * Appends the \p pNumBytes bytes read into the intermediate read buffer (see pollReadBuffer()) to the (actual) read buffer.
 *
 * If continuous polling is enabled (see init() / close()), initiates the next asynchronous read by calling pollReadBuffer().
 *
 * If \p pErrorCode signals an error (other than the socket being cancelled), the error gets logged (see Logger)
 * and the current polling error count gets incremented. Continuous polling gets disabled automatically
 * as soon as this error count exceeds a fixed maximum threshold (see \ref maxBufferErrorCount).
 *
 * \param pErrorCode Result/error code of the handled async read.
 * \param pNumBytes Number of successfully transferred bytes.
 */
void SerialPortWrapper::handleAsyncRead(const boost::system::error_code& pErrorCode, const std::size_t pNumBytes)
{
    if (pErrorCode.value() != boost::system::errc::success && pErrorCode.value() != boost::system::errc::operation_canceled)
    {
        if (intermediateReadBuffer.size() > pNumBytes)
            intermediateReadBuffer.resize(pNumBytes);

        Logger::logError("Exception while reading from serial port \"" + port + "\": " + pErrorCode.message());

        if (++bufferErrorCount > maxBufferErrorCount)
        {
            pollData.store(false);
            Logger::logCritical("Exceeded maximum error count while polling serial port \"" + port + "\". Stopping...");

            //Set stopped flag already here and notify about "new data" after that,
            //such that read operations can check it in order to not get stuck
            pollDataStopped.store(true);
            pollDataStopped.notify_one();

            //Need to resolve potential waiting in read functions even if there is no new data
            if (pNumBytes == 0)
            {
                {
                    const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
                    (void)bufferLock;
                    newData = true;
                }
                newDataCondVar.notify_one();
                return;
            }

        }
    }

    if (pNumBytes > 0)
    {
        {
            const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
            (void)bufferLock;

            readBuffer.insert(readBuffer.end(), intermediateReadBuffer.begin(), intermediateReadBuffer.end());

            intermediateReadBuffer.clear();

            newData = true;
        }
        newDataCondVar.notify_one();
    }

    if (pollData.load())
        pollReadBuffer();
    else
    {
        pollDataStopped.store(true);
        pollDataStopped.notify_one();
    }
}

/// \endcond INTERNAL
