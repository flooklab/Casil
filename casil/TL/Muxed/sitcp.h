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

#ifndef CASIL_TL_SITCP_H
#define CASIL_TL_SITCP_H

#include <casil/TL/muxedinterface.h>

#include <casil/auxil.h>
#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/TL/CommonImpl/tcpsocketwrapper.h>
#include <casil/TL/CommonImpl/udpsocketwrapper.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace casil
{

namespace TL
{

/*!
 * \brief %Interface to connect to the basil bus on an FPGA that runs the \e %SiTCP library for communication.
 *
 * ...
 *
 * Protocol information:
 *
 * %UDP (RBCP) Header+Data
 *
 * \code{.unparsed}
 *
 * Bit 7         Bit 0
 * +-----------------+
 * |  Ver.  |  Type  |
 * +-----------------+
 * |   CMD  |  FLAG  |
 * +-----------------+
 * |        ID       |
 * +-----------------+
 * |   Data Length   |
 * +-----------------+
 * | Address [31:24] |
 * +-----------------+
 * | Address [23:16] |
 * +-----------------+
 * | Address [15:8]  |
 * +-----------------+
 * | Address [7:0]   |
 * +-----------------+
 * |      Data 0     |
 * +-----------------+
 * |      Data 1     |
 * +-----------------+
 * |       ...       |
 * +-----------------+
 * |     Data N-1    |
 * +-----------------+
 * |      Data N     | (N max. 255)
 * +-----------------+
 *
 * \endcode
 *
 * CMD Field
 *
 * \code{.unparsed}
 *
 * +-----+------------+-------------+
 * | BIT |    Name    | Description |
 * +--------------------------------+
 * |  3  |   Access   | Bus Access  |
 * +--------------------------------+
 * |  2  |    R/W     | 0:Wr,1:Read |
 * +--------------------------------+
 * |  1  |  Reserved  |   Always 0  |
 * +--------------------------------+
 * |  0  |  Reserved  |   Always 0  |
 * +-----+------------+-------------+
 *
 * \endcode
 *
 * FLAG Field
 *
 * \code{.unparsed}
 *
 * +-----+------------+-------------+
 * | BIT |    Name    | Description |
 * +--------------------------------+
 * |  3  |  REQ/ACK   | 0:Req,1:Ack |
 * +--------------------------------+
 * |  2  |  Reserved  |   Always 0  |
 * +--------------------------------+
 * |  1  |  Reserved  |   Always 0  |
 * +--------------------------------+
 * |  0  |    Error   |   0:Normal  |
 * |     |            | 1:Bus Error |
 * +-----+------------+-------------+
 *
 * \endcode
 *
 * %TCP to BUS Header+Data
 *
 * \code{.unparsed}
 *
 * Bit 7         Bit 0
 * +-----------------+
 * |   Length [7:0]  |
 * +-----------------+
 * |   Length [15:8] |
 * +-----------------+
 * | Address [7:0]   |
 * +-----------------+
 * | Address [15:8]  |
 * +-----------------+
 * | Address [23:16] |
 * +-----------------+
 * | Address [31:24] |
 * +-----------------+
 * |      Data 0     |
 * +-----------------+
 * |      Data 1     |
 * +-----------------+
 * |       ...       |
 * +-----------------+
 * |  Data Length-1  |
 * +-----------------+
 * |   Data Length   | (Length max. 65529)
 * +-----------------+
 *
 * \endcode
 *
 * %TCP to BUS reset sequence (in case of status invalid): 65535 * 0xFF + 6 * 0x00
 *
 * \todo doc...
 */
class SiTCP final : public MuxedInterface
{
public:
    SiTCP(std::string pName, LayerConfig pConfig);
    ~SiTCP() override;
    //
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) override;
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;
    //
    void resetFifo();
    void resetFifoMod32();
    std::size_t getFifoSize() const;
    std::vector<std::uint8_t> getFifoData(int pSize = -1);

private:
    bool initImpl() override;
    bool closeImpl() override;
    //
    void enableTcpToBus();
    //
    void pollFifo();
    //
    std::vector<std::uint8_t> readSingle(std::uint32_t pAddr, std::uint8_t pSize);
    void writeSingle(std::uint32_t pAddr, const std::vector<std::uint8_t>& pData);
    //
    std::optional<std::vector<std::uint8_t>> doSingleRBCPOperation(std::uint32_t pAddr, const std::variant<
                                                                   std::uint8_t,
                                                                   std::reference_wrapper<const std::vector<std::uint8_t>>> pSizeOrData);

private:
    const std::string hostName;
    const int udpPort;
    const int tcpPort;
    //
    const bool useTcp;
    const bool useTcpToBus;
    //
    const double connectTimeoutSecs;
    const std::chrono::milliseconds connectTimeout;
    //
    CommonImpl::UDPSocketWrapper udpSocketWrapper;
    const std::unique_ptr<CommonImpl::TCPSocketWrapper> tcpSocketWrapperPtr;
    //
    std::thread fifoThread;
    std::deque<std::uint8_t> fifoBuffer;
    mutable std::mutex fifoMutex;
    std::mutex tcpSocketMutex;
    std::atomic_flag wantLockTCPSocket;     //Need this to trigger FIFO thread to yield() to enable quicker/"fairer" access to tcpSocketMutex
    std::atomic_bool pollFIFO;
    std::atomic_size_t fifoErrorCount;
    //
    std::uint8_t rbcpId;

public:
    static constexpr std::uint64_t baseAddrDataLimit = 0x100000000;
    static constexpr std::uint64_t baseAddrFIFOLimit = 0x200000000;

private:
    static constexpr std::uint8_t rbcpVerType = 0xFF;   //Version/type byte of RBCP header
    static constexpr std::uint8_t rbcpCmdWr = 0x80;     //Write request value of CMD/FLAG byte of RBCP header
    static constexpr std::uint8_t rbcpCmdRd = 0xC0;     //Read request value of CMD/FLAG byte of RBCP header
    static constexpr std::uint8_t rbcpMaxSize = 255;    //Maximum number of data bytes
    //
    static constexpr std::chrono::milliseconds udpTimeout {1000};
    static constexpr int udpRetransmitCnt = 3;
    static constexpr std::chrono::milliseconds tcpReadoutInterval = Auxil::getChronoMilliSecs(0.05);
    //
    static constexpr std::size_t maxFIFOErrorCount = 10;

    CASIL_REGISTER_INTERFACE_H("SiTCP")
};

} // namespace TL

} // namespace casil

#endif // CASIL_TL_SITCP_H
