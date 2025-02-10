/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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

#ifndef CASIL_LAYERS_TL_TCP_H
#define CASIL_LAYERS_TL_TCP_H

#include <casil/TL/directinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::TL
{

namespace CommonImpl { class TCPSocketWrapper; }

/*!
 * \brief %Interface for network communication using the \e Transmission \e Control \e Protocol (%TCP).
 *
 * Use this interface for network connections (typically over Ethernet) to hardware using the %TCP protocol.
 */
class TCP final : public DirectInterface
{
public:
    TCP(std::string pName, LayerConfig pConfig);    ///< Constructor.
    ~TCP() override;                                ///< Default destructor.
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
    const std::string hostName;                     ///< Host name of the remote endpoint.
    const int port;                                 ///< Used network port.
    const std::string readTermination;              ///< Read termination to detect end of read data stream.
    const std::string writeTermination;             ///< Write termination to append to written data.
    //
    const std::unique_ptr<CommonImpl::TCPSocketWrapper> socketWrapperPtr;   ///< Detailed %TCP socket logic wrapper.

    CASIL_REGISTER_INTERFACE_H("TCP")
};

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_TCP_H
