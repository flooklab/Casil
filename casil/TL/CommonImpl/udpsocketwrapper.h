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

#ifndef CASIL_TL_COMMONIMPL_UDPSOCKETWRAPPER_H
#define CASIL_TL_COMMONIMPL_UDPSOCKETWRAPPER_H

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

namespace TL
{

namespace CommonImpl
{

class UDPSocketWrapper
{
public:
    UDPSocketWrapper(std::string pHostName, int pPort);
    UDPSocketWrapper(const UDPSocketWrapper&) = delete;
    UDPSocketWrapper(UDPSocketWrapper&&) = default;
    //
    UDPSocketWrapper& operator=(UDPSocketWrapper) = delete;
    UDPSocketWrapper& operator=(UDPSocketWrapper&&) = delete;
    //
    std::vector<std::uint8_t> read(std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                   std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    std::vector<std::uint8_t> readMax(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                      std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    void write(const std::vector<std::uint8_t>& pData, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
               std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    //
    bool readBufferEmpty() const;
    void clearReadBuffer();
    //
    void init(std::chrono::milliseconds pConnectTimeout = std::chrono::milliseconds::zero(),
              std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    void close();

private:
    const std::string hostName;
    const int port;
    //
    boost::asio::ip::udp::socket socket;
    //
    static constexpr std::size_t readBufferSize = 65527;
    std::array<std::uint8_t, readBufferSize> readBuffer;
};

} // namespace CommonImpl

} // namespace TL

} // namespace casil

#endif // CASIL_TL_COMMONIMPL_UDPSOCKETWRAPPER_H
