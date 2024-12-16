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

#ifndef CASIL_LAYERS_TL_COMMONIMPL_SERIALPORTWRAPPER_H
#define CASIL_LAYERS_TL_COMMONIMPL_SERIALPORTWRAPPER_H

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

namespace Layers::TL
{

namespace CommonImpl
{

/*!
 * \brief Wrapper class around the serial port interface of the Boost %ASIO library.
 *
 * \todo Detailed doc
 */
class SerialPortWrapper
{
public:
    SerialPortWrapper(std::string pPort, const std::string& pReadTermination, const std::string& pWriteTermination, int pBaudRate);
                                                                ///< Constructor.
    SerialPortWrapper(const SerialPortWrapper&) = delete;       ///< Deleted copy constructor.
    SerialPortWrapper(SerialPortWrapper&&) = delete;            ///< Deleted move constructor.
    ~SerialPortWrapper();                                       ///< Destructor.
    //
    SerialPortWrapper& operator=(SerialPortWrapper) = delete;   ///< Deleted copy assignment operator.
    SerialPortWrapper& operator=(SerialPortWrapper&&) = delete; ///< Deleted move assignment operator.
    //
    std::vector<std::uint8_t> read(int pSize);                  ///< Read an amount of bytes from the read buffer, or until read termination.
    std::vector<std::uint8_t> readMax(int pSize);               ///< Read maximally some amount of bytes from the read buffer.
    void write(const std::vector<std::uint8_t>& pData);         ///< Write data to the port (automatically terminated).
    //
    bool readBufferEmpty() const;                               ///< Check if the read buffer is empty.
    void clearReadBuffer();                                     ///< Clear the current contents of the read buffer.
    //
    void init();                                                ///< Open the serial port and start a read buffer polling thread.
    void close();                                               ///< Stop the read buffer polling thread and close the serial port.

private:
    void pollReadBuffer();                                                                      ///< \brief Issue an async read to poll the
                                                                                                ///  serial port (handler is handleAsyncRead()).
    void handleAsyncRead(const boost::system::error_code& pErrorCode, std::size_t pNumBytes);   ///< \brief Fill read buffer from single poll
                                                                                                ///  by pollReadBuffer() and issue next poll.

private:
    const std::string port;                                 ///< %Serial port identifier (e.g. device file).
    const std::vector<std::uint8_t> readTermination;        ///< Read termination to detect end of read data stream.
    const std::size_t readTerminationLength;                ///< Number of read termination bytes.
    const std::vector<std::uint8_t> writeTermination;       ///< Write termination to append to written data.
    const std::size_t writeTerminationLength;               ///< Number of write termination bytes.
    const int baudRate;                                     ///< Baud rate setting.
    //
    boost::asio::serial_port serialPort;                    ///< %Serial port.
    //
    std::vector<std::uint8_t> readBuffer;                   ///< Buffer for incoming data.
    std::vector<std::uint8_t> intermediateReadBuffer;       ///< Intermediate read buffer to facilitate the asynchronous polling.
    mutable std::mutex readBufferMutex;                     ///< Mutex for the read buffer (\ref readBuffer).
    std::atomic_bool pollData;                              ///< Flag to control/stop the read buffer polling.
    std::atomic_bool pollDataStopped;                       ///< Flag to signal stopped read buffer polling (last handler finished).
    bool newData;                                           ///< New read buffer data available from polling handler.
    std::condition_variable newDataCondVar;                 ///< Condition variable for \ref newData.
    std::atomic_size_t bufferErrorCount;                    ///< Current error count of the read buffer polling handler.

private:
    static constexpr std::size_t maxBufferErrorCount = 10;  ///< Maximum error count for the read buffer polling before it stops itself.
};

} // namespace CommonImpl

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_COMMONIMPL_SERIALPORTWRAPPER_H
