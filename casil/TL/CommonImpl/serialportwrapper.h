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

#ifndef CASIL_TL_COMMONIMPL_SERIALPORTWRAPPER_H
#define CASIL_TL_COMMONIMPL_SERIALPORTWRAPPER_H

#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

namespace casil
{

namespace TL
{

namespace CommonImpl
{

class SerialPortWrapper
{
public:
    SerialPortWrapper(std::string pPort, const std::string& pReadTermination, const std::string& pWriteTermination, int pBaudRate);
    SerialPortWrapper(const SerialPortWrapper&) = delete;
    SerialPortWrapper(SerialPortWrapper&&) = delete;
    ~SerialPortWrapper();
    //
    SerialPortWrapper& operator=(SerialPortWrapper) = delete;
    SerialPortWrapper& operator=(SerialPortWrapper&&) = delete;
    //
    std::vector<std::uint8_t> read(int pSize);
    std::vector<std::uint8_t> readMax(int pSize);
    void write(const std::vector<std::uint8_t>& pData);
    //
    bool readBufferEmpty() const;
    void clearReadBuffer();
    //
    void init();
    void close();

private:
    void pollReadBuffer();
    void handleAsyncRead(const boost::system::error_code& pErrorCode, std::size_t pNumBytes);

private:
    const std::string port;
    const std::vector<std::uint8_t> readTermination;
    const std::size_t readTerminationLength;
    const std::vector<std::uint8_t> writeTermination;
    const std::size_t writeTerminationLength;
    const int baudRate;
    //
    boost::asio::serial_port serialPort;
    //
    std::vector<std::uint8_t> readBuffer;
    std::vector<std::uint8_t> intermediateReadBuffer;
    mutable std::mutex readBufferMutex;
    std::atomic_bool pollData;
    std::atomic_bool pollDataStopped;
    bool newData;
    std::condition_variable newDataCondVar;
    std::atomic_size_t bufferErrorCount;

private:
    static constexpr std::size_t maxBufferErrorCount = 10;
};

} // namespace CommonImpl

} // namespace TL

} // namespace casil

#endif // CASIL_TL_COMMONIMPL_SERIALPORTWRAPPER_H
