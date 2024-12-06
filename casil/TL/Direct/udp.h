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

#ifndef CASIL_TL_UDP_H
#define CASIL_TL_UDP_H

#include <casil/TL/directinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/TL/CommonImpl/udpsocketwrapper.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace TL
{

/*!
 * \brief %Interface for network communication using the \e User \e Datagram \e Protocol (%UDP).
 *
 * \todo Detailed doc
 */
class UDP final : public DirectInterface
{
public:
    UDP(std::string pName, LayerConfig pConfig);
    ~UDP() override = default;
    //
    std::vector<std::uint8_t> read(int pSize = -1) override;
    void write(const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    const std::string hostName;
    const int port;
    //
    CommonImpl::UDPSocketWrapper socketWrapper;

    CASIL_REGISTER_INTERFACE_H("UDP")
};

} // namespace TL

} // namespace casil

#endif // CASIL_TL_UDP_H
