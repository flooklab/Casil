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

#ifndef CASIL_LAYERS_TL_COMMONIMPL_UDPSOCKETWRAPPER_H
#define CASIL_LAYERS_TL_COMMONIMPL_UDPSOCKETWRAPPER_H

#include <boost/asio/ip/udp.hpp>

#include <array>
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
 * \brief Wrapper class around the %UDP network socket of the Boost %ASIO library.
 *
 * Wraps the %UDP socket by providing basic synchronous connect/read/write functionality with
 * the option to use timeouts (abstracting necessary internal <em>a</em>synchronous calls etc.).
 */
class UDPSocketWrapper
{
public:
    UDPSocketWrapper(std::string pHostName, int pPort);         ///< Constructor.
    UDPSocketWrapper(const UDPSocketWrapper&) = delete;         ///< Deleted copy constructor.
    UDPSocketWrapper(UDPSocketWrapper&&) = default;             ///< Default move constructor.
    //
    UDPSocketWrapper& operator=(UDPSocketWrapper) = delete;     ///< Deleted copy assignment operator.
    UDPSocketWrapper& operator=(UDPSocketWrapper&&) = delete;   ///< Deleted move assignment operator.
    //
    std::vector<std::uint8_t> read(std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                   std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                                                ///< Receive a single datagram from the socket.
    std::vector<std::uint8_t> readMax(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                      std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
                                                                                                ///< \brief Receive maximally some amount of
                                                                                                ///  bytes of a single datagram from the socket.
    void write(const std::vector<std::uint8_t>& pData, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
               std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);           ///< Send a single datagram over the socket.
    //
    bool readBufferEmpty() const;                               ///< Check if no incoming datagrams are available on the socket.
    void clearReadBuffer();                                     ///< Read remaining datagrams from the socket and discard them.
    //
    void init(std::chrono::milliseconds pConnectTimeout = std::chrono::milliseconds::zero(),
              std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);            ///< Connect the %UDP socket.
    void close();                                                                               ///< Disconnect the %UDP socket.

private:
    const std::string hostName;                             ///< Host name of the remote endpoint.
    const int port;                                         ///< Used network port.
    //
    boost::asio::ip::udp::socket socket;                    ///< %UDP socket.
    //
    static constexpr std::size_t readBufferSize = 65527;    ///< Maximum %UDP datagram payload size.
    std::array<std::uint8_t, readBufferSize> readBuffer;    ///< Buffer for incoming datagrams.
};

} // namespace CommonImpl

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_COMMONIMPL_UDPSOCKETWRAPPER_H
