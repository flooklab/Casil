/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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
//
//  This file has been directly adapted from the corresponding code of
//  the basil software, which itself is covered by the following license:
//
//      Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn
//
//      All rights reserved.
//
//      Redistribution and use in source and binary forms, with or without
//      modification, are permitted provided that the following conditions are
//      met:
//
//       *  Redistributions of source code must retain the above copyright notice,
//          this list of conditions and the following disclaimer.
//
//       *  Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//
//       *  Neither the name of the copyright holder nor the names of its
//          contributors may be used to endorse or promote products derived from
//          this software without specific prior written permission.
//
//      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef CASIL_LAYERS_TL_SISIM_H
#define CASIL_LAYERS_TL_SISIM_H

#include <casil/TL/muxedinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace casil
{

namespace Layers::TL
{

namespace CommonImpl { class TCPSocketWrapper; }

/*!
 * \brief %Interface to \e virtually connect to the basil bus in a \e cocotb co-simulation.
 *
 * Use this interface as a replacement for e.g. SiTCP when running the FPGA firmware as a co-simulation
 * using the provided simulation/test code, which uses \e cocotb (see also https://www.cocotb.org/).
 *
 * The interface talks to the simulation over a %TCP/IP socket, typically on the loopback interface (localhost).
 * From there the communication is forwarded/injected (in)to the simulated "basil bus" signals.
 */
class SiSim final : public MuxedInterface
{
public:
    enum class MessageType : std::uint8_t;

public:
    SiSim(std::string pName, LayerConfig pConfig);      ///< Constructor.
    ~SiSim() override;                                  ///< Default destructor.
    //
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) override;
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;
    //
    static std::vector<std::uint8_t> createWriteRequest(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData);
                                                                                    ///< Create a write request to send to simulation.
    static std::vector<std::uint8_t> createReadRequest(std::uint64_t pAddr, std::uint32_t pSize);
                                                                                    ///< Create a read request to send to simulation.
    static std::vector<std::uint8_t> createReadResponse(const std::vector<std::uint8_t>& pData);
                                                                                    ///< Create a read response to send back to interface.
    //
    static std::tuple<std::uint64_t, std::vector<std::uint8_t>> parseWriteRequest(const std::vector<std::uint8_t>& pData);
                                                                                    ///< Parse a write request as received from interface.
    static std::tuple<std::uint64_t, std::uint32_t> parseReadRequest(const std::vector<std::uint8_t>& pData);
                                                                                    ///< Parse a read request as received from interface.
    static std::vector<std::uint8_t> parseReadResponse(const std::vector<std::uint8_t>& pData);
                                                                                    ///< Parse a read response as received from simulation.
    //
    static MessageType parseMessageType(std::uint8_t pTypeByte);                    ///< Parse message type from first message byte.
    static std::uint32_t parseMessageSize(const std::vector<std::uint8_t>& pData);  ///< Parse payload size from message size bytes.

private:
    static std::vector<std::uint8_t> createMessage(MessageType pType, const std::vector<std::uint8_t>& pData);
                                                                                    ///< \brief Create message to send between
                                                                                    ///  interface/simulation or vice versa.
    static std::uint8_t createMessageTypeByte(MessageType pType);                   ///< Create first message byte for a certain type.
    static std::vector<std::uint8_t> createMessageSizeSeq(std::uint32_t pSize);     ///< Create message size bytes according to payload size.
    //
    bool initImpl() override;
    bool closeImpl() override;

private:
    const std::string hostName;     ///< Host name of the remote endpoint.
    const std::uint64_t port;       ///< Used network port.
    //
    const std::uint64_t maxConnectRetries;              ///< Maximum retry attempts for establishing the socket connection (for init()).
    //
    const std::unique_ptr<CommonImpl::TCPSocketWrapper> socketWrapperPtr;   ///< Detailed %TCP socket logic wrapper.

public:
    /*!
     * \brief Type of a message that is passed between interface/simulation or vice versa.
     *
     * Determines the type and thus format (and direction) of a passed message.
     */
    enum class MessageType : std::uint8_t
    {
        WriteRequest = 0,   ///< Write request from interface to simulation.
        ReadRequest = 1,    ///< Read request from interface to simulation.
        ReadResponse = 2    ///< Read response from simulation back to interface.
    };

    CASIL_REGISTER_INTERFACE_H("SiSim")
};

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_SISIM_H
