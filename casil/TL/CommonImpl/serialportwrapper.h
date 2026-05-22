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

/// \cond INTERNAL
namespace CommonImpl
{

/*!
 * \brief Wrapper class around the serial port interface of the Boost %ASIO library.
 *
 * Wraps the serial port by providing basic synchronous read/write functionality,
 * abstracting internal <em>a</em>synchronous calls etc.
 */
class SerialPortWrapper
{
public:
    enum class PortParity : std::uint8_t;
    enum class PortStopBits : std::uint8_t;
    enum class PortFlowControl : std::uint8_t;

public:
    SerialPortWrapper(std::string pPort, const std::string& pReadTermination, const std::string& pWriteTermination, unsigned int pBaudRate,
                      unsigned int pCharacterSize, PortParity pParity, PortStopBits pStopBits, PortFlowControl pFlowControl);
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
    void init();                                                ///< Open the serial port and start continuous read buffer polling.
    void close();                                               ///< Stop the continuous read buffer polling and close the serial port.

private:
    void pollReadBuffer();                                                                  ///< \brief Issue an async read to poll the
                                                                                            ///  serial port (handler is handleAsyncRead()).
    void handleAsyncRead(const boost::system::error_code& pErrorCode, std::size_t pNumBytes);   ///< \brief Fill read buffer from single poll
                                                                                                ///  by pollReadBuffer() and issue next poll.

private:
    const std::string port;                                 ///< %Serial port identifier (e.g. device file).
    const std::vector<std::uint8_t> readTermination;        ///< Read termination to detect end of read data stream.
    const std::size_t readTerminationLength;                ///< Number of read termination bytes.
    const std::vector<std::uint8_t> writeTermination;       ///< Write termination to append to written data.
    const std::size_t writeTerminationLength;               ///< Number of write termination bytes.
    //
    const unsigned int baudRate;                            ///< Baud rate setting.
    const unsigned int characterSize;                       ///< Character size setting.
    const PortParity parityOption;                          ///< Parity setting.
    const PortStopBits stopBitsOption;                      ///< Stop bit setting.
    const PortFlowControl flowControlOption;                ///< Flow control setting.
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

public:
    /*!
     * \brief Parity mode of the serial communication.
     *
     * Select whether to use a parity bit per character transmission and whether to use even or odd parity.
     */
    enum class PortParity : std::uint8_t
    {
        None = 0,           ///< No parity bit.
        Odd = 1,            ///< Parity bit with odd parity (i.e. odd number of <tt>1</tt>-bits)
        Even = 2            ///< Parity bit with even parity (i.e. even number of <tt>1</tt>-bits)
    };
    /*!
     * \brief Number of stop bits to use for the serial communication.
     *
     * Select which stop bit timing to use for every character transmission termination.
     */
    enum class PortStopBits : std::uint8_t
    {
        One = 0,            ///< One stop bit.
        OnePointFive = 1,   ///< One and a half stop bits.
        Two = 2             ///< Two stop bits.
    };
    /*!
     * \brief Type of flow control to use for the serial communication.
     *
     * Select whether to use flow control and which type (software-based or hardware-based handshaking).
     */
    enum class PortFlowControl : std::uint8_t
    {
        None = 0,           ///< No flow control.
        Software = 1,       ///< Flow control handled in software.
        Hardware = 2        ///< Flow control handled in hardware.
    };
};

} // namespace CommonImpl
/// \endcond INTERNAL

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_COMMONIMPL_SERIALPORTWRAPPER_H
