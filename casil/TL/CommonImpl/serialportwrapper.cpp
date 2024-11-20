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

#include <casil/TL/CommonImpl/serialportwrapper.h>

#include <casil/asio.h>
#include <casil/bytes.h>
#include <casil/logger.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/system_error.hpp>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

using casil::TL::CommonImpl::SerialPortWrapper;

//

SerialPortWrapper::SerialPortWrapper(std::string pPort, const std::string& pReadTermination, const std::string& pWriteTermination,
                                     const int pBaudRate) :
    port(std::move(pPort)),
    readTermination(Bytes::byteVecFromStr(pReadTermination)),
    readTerminationLength(readTermination.size()),
    writeTermination(Bytes::byteVecFromStr(pWriteTermination)),
    writeTerminationLength(writeTermination.size()),
    baudRate(pBaudRate),
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

std::vector<std::uint8_t> SerialPortWrapper::read(const int pSize)
{
    std::unique_lock<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    auto waitForNewData = [this, &bufferLock]() -> void
    {
        newData = false;
        newDataCondVar.wait(bufferLock, [this](){ return newData; });
    };

    if (pSize == -1)
    {
        auto termPos = std::search(readBuffer.begin(), readBuffer.end(), readTermination.begin(), readTermination.end());

        while (termPos == readBuffer.end())
        {
            waitForNewData();
            termPos = std::search(readBuffer.begin(), readBuffer.end(), readTermination.begin(), readTermination.end());
        }

        std::vector<std::uint8_t> retVal(readBuffer.begin(), termPos);

        readBuffer.erase(readBuffer.begin(), termPos + readTerminationLength);

        return retVal;
    }
    else if (pSize > 0)
    {
        while (std::cmp_less(readBuffer.size(), pSize))
            waitForNewData();

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

std::vector<std::uint8_t> SerialPortWrapper::readMax(const int pSize)
{
    if (pSize > 0)
    {
        std::unique_lock<std::mutex> bufferLock(readBufferMutex);
        (void)bufferLock;

        auto waitForNewData = [this, &bufferLock]() -> void
        {
            newData = false;
            newDataCondVar.wait(bufferLock, [this](){ return newData; });
        };

        while (readBuffer.size() == 0)
            waitForNewData();

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

void SerialPortWrapper::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        boost::asio::write(serialPort, boost::asio::buffer(pData.data(), pData.size()));
        boost::asio::write(serialPort, boost::asio::buffer(writeTermination));
    }
    catch (const boost::system::system_error& exc)
    {
        throw std::runtime_error(std::string("Exception while writing to serial port: ") + exc.what());
    }
}

//

bool SerialPortWrapper::readBufferEmpty() const
{
    const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    return readBuffer.empty();
}

void SerialPortWrapper::clearReadBuffer()
{
    const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
    (void)bufferLock;

    readBuffer.clear();
}

//

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

    pollData.store(true);
    pollDataStopped.store(false);
    bufferErrorCount.store(0);

    pollReadBuffer();
}

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

void SerialPortWrapper::pollReadBuffer()
{
    boost::asio::async_read(serialPort, boost::asio::dynamic_buffer(intermediateReadBuffer), boost::asio::transfer_at_least(1),
                            std::bind(&SerialPortWrapper::handleAsyncRead, this, std::placeholders::_1 ,std::placeholders::_2));
}

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
        }
    }

    if (pNumBytes > 0)
    {
        const std::lock_guard<std::mutex> bufferLock(readBufferMutex);
        (void)bufferLock;

        readBuffer.insert(readBuffer.end(), intermediateReadBuffer.begin(), intermediateReadBuffer.end());

        intermediateReadBuffer.clear();

        newData = true;
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
