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

#ifndef CASIL_TL_COMMONIMPL_TCPSOCKETWRAPPER_H
#define CASIL_TL_COMMONIMPL_TCPSOCKETWRAPPER_H

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

namespace TL
{

namespace CommonImpl
{

class TCPSocketWrapper
{
public:
    TCPSocketWrapper(std::string pHostName, int pPort, std::string pReadTermination, const std::string& pWriteTermination);
    TCPSocketWrapper(const TCPSocketWrapper&) = delete;
    TCPSocketWrapper(TCPSocketWrapper&&) = default;
    //
    TCPSocketWrapper& operator=(TCPSocketWrapper) = delete;
    TCPSocketWrapper& operator=(TCPSocketWrapper&&) = delete;
    //
    std::vector<std::uint8_t> read(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                   std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    std::vector<std::uint8_t> readMax(int pSize, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
                                      std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    void write(const std::vector<std::uint8_t>& pData, std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
               std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    //
    bool readBufferEmpty() const;
    void clearReadBuffer();
    //
    void init(std::chrono::milliseconds pTimeout = std::chrono::milliseconds::zero(),
              std::optional<std::reference_wrapper<bool>> pTimedOut = std::nullopt);
    void close();

private:
    const std::string hostName;
    const int port;
    const std::string readTermination;
    const std::size_t readTerminationLength;
    const std::vector<std::uint8_t> writeTermination;
    const std::size_t writeTerminationLength;
    //
    boost::asio::ip::tcp::socket socket;
    //
    std::vector<std::uint8_t> readBuffer;
};

} // namespace CommonImpl

} // namespace TL

} // namespace casil

#endif // CASIL_TL_COMMONIMPL_TCPSOCKETWRAPPER_H
