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

#ifndef CASIL_LAYERS_HL_SITCPFIFO_H
#define CASIL_LAYERS_HL_SITCPFIFO_H

#include <casil/HL/muxeddriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace casil
{

namespace Layers::TL { class SiTCP; }

namespace Layers::HL
{

/*!
 * \brief Special driver to access the FIFO of the \ref TL::SiTCP "SiTCP" interface.
 *
 * \todo Detailed doc
 */
class SiTCPFifo final : public MuxedDriver
{
public:
    SiTCPFifo(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);   ///< Constructor.
    ~SiTCPFifo() override = default;                                                    ///< Default destructor.
    //
    std::size_t operator[](std::string_view pRegName);                  ///< Register-like access to some functions.
    //
    void reset() override;                                              ///< Reset the FIFO.
    //
    std::uint8_t getVersion() const;                                    ///< Get the pseudo FIFO module version.
    //
    std::size_t getFifoSize() const;                                    ///< Get the FIFO size in number of bytes.
    std::vector<std::uint32_t> getFifoData() const;                     ///< Read the FIFO content as sequence of 32 bit unsigned integers.
    void setFifoData(const std::vector<std::uint32_t>& pData) const;    ///< Write a sequence of 32 bit unsigned integers to the FIFO.

private:
    bool initImpl() override;                           ///< Initialize the driver by doing nothing.
    bool closeImpl() override;                          ///< Close the driver by doing nothing.

private:
    using SiTCP = TL::SiTCP;                            ///< \copybrief TL::SiTCP
    SiTCP& siTcpIntf;                                   ///< The \ref MuxedDriver::interface "interface" instance casted to needed SiTCP type.

private:
    static constexpr std::uint8_t pseudoVersion = 0;    ///< Need to provide a fake version of the non-existent firmware module.

    CASIL_REGISTER_DRIVER_H("SiTCPFifo")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_SITCPFIFO_H
