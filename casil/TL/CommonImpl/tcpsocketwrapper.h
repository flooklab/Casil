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

#ifndef CASIL_LAYERS_TL_COMMONIMPL_TCPSOCKETWRAPPER_H
#define CASIL_LAYERS_TL_COMMONIMPL_TCPSOCKETWRAPPER_H

#include <boost/asio/ip/tcp.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::TL
{

namespace CommonImpl
{

/*!
 * \brief Wrapper class around the %TCP network socket of the Boost %ASIO library.
 *
 * Wraps the %TCP socket by providing basic synchronous connect/read/write functionality with
 * the option to use timeouts (abstracting necessary internal <em>a</em>synchronous calls etc.).
 */
class TCPSocketWrapper
{
public:
    TCPSocketWrapper(std::string pHostName, int pPort, std::string pReadTermination, const std::string& pWriteTermination);
                                                                ///< Constructor.
    TCPSocketWrapper(const TCPSocketWrapper&) = delete;         ///< Deleted copy constructor.
    TCPSocketWrapper(TCPSocketWrapper&&) = default;             ///< Default move constructor.
    //
    TCPSocketWrapper& operator=(TCPSocketWrapper) = delete;     ///< Deleted copy assignment operator.
    TCPSocketWrapper& operator=(TCPSocketWrapper&&) = delete;   ///< Deleted move assignment operator.
    //
    std::vector<std::uint8_t> read(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                   std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                                    ///< \brief Read an amount of bytes from the socket,
                                                                                    ///  or until read termination.
    std::vector<std::uint8_t> readMax(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                      std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                                    ///< Read maximally some amount of bytes from the socket.
    void write(const std::vector<std::uint8_t>& pData, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
               std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                                    ///< Write data to the socket (automatically terminated).
    //
    bool readBufferEmpty() const;                               ///< Check if the read buffer is empty (and no remaining data to be read).
    void clearReadBuffer();                                     ///< Read remaining data from the socket and then clear the read buffer contents.
    //
    void init(std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
              std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);    ///< Connect the %TCP socket.
    void close();                                                                       ///< Disconnect the %TCP socket.

private:
    const std::string hostName;                         ///< Host name of the remote endpoint.
    const int port;                                     ///< Used network port.
    const std::string readTermination;                  ///< Read termination to detect end of read data stream.
    const std::size_t readTerminationLength;            ///< Number of read termination characters.
    const std::vector<std::uint8_t> writeTermination;   ///< Write termination to append to written data.
    const std::size_t writeTerminationLength;           ///< Number of read termination characters/bytes.
    //
    boost::asio::ip::tcp::socket socket;                ///< %TCP socket.
    //
    std::vector<std::uint8_t> readBuffer;               ///< Buffer for incoming data.
};

} // namespace CommonImpl

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_COMMONIMPL_TCPSOCKETWRAPPER_H
