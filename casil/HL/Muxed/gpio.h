/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#ifndef CASIL_LAYERS_HL_GPIO_H
#define CASIL_LAYERS_HL_GPIO_H

#include <casil/HL/registerdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <boost/dynamic_bitset_fwd.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief %Driver for the \e gpio firmware module.
 *
 * Enables to configure/set/read the state of the individual General Purpose Input/Output (%GPIO) pins/wires of the firmware module.
 *
 * Available registers:
 *
 * <table>
 * <tr><th>Name</th><th>\ref RegisterDescr::DataType "DataType"</th><th>\ref RegisterDescr::AccessMode "AccessMode"</th>
 * <th>Size</th><th>Description</th></tr>
 * <tr><td>\c RESET</td>    <td>Value</td>    <td>WriteOnly</td><td>8</td>          <td>Soft reset active on write to address.</td></tr>
 * <tr><td>\c VERSION</td>  <td>Value</td>    <td>ReadOnly</td> <td>8</td>          <td>Firmware module version.</td></tr>
 * <tr><td>\c INPUT</td>    <td>ByteArray</td><td>ReadOnly</td> <td>\c IO_BYTES</td><td>Read input state of IO pins.</td></tr>
 * <tr><td>\c OUTPUT</td>   <td>ByteArray</td><td>ReadWrite</td><td>\c IO_BYTES</td><td>Assigned output state of IO pins.</td></tr>
 * <tr><td>\c OUTPUT_EN</td><td>ByteArray</td><td>ReadWrite</td><td>\c IO_BYTES</td><td>Direction of IO pins (0: input, 1: output).</td></tr>
 * </table>
 *
 * Here \c IO_BYTES is the number of full register bytes occupied by the configured IO bits (see "size" in GPIO::GPIO()).
 */
class GPIO final : public RegisterDriver
{
public:
    GPIO(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);                    ///< Constructor.
    ~GPIO() override = default;                                                                     ///< Default destructor.
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;        ///< Get the \c INPUT register.
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override;     ///< Set the \c OUTPUT register.
    //
    std::uint64_t getSize() const;                                                                  ///< Get the number of IO bits.
    //
    std::vector<std::uint8_t> getOutputEn();                                                        ///< Get the \c OUTPUT_EN register.
    void setOutputEn(const std::vector<std::uint8_t>& pEnable);                                     ///< Set the \c OUTPUT_EN register.
    //
    boost::dynamic_bitset<> bitsetFromBytes(const std::vector<std::uint8_t>& pBytes) const;         ///< Convert IO register bytes to a bitset.
    std::vector<std::uint8_t> bytesFromBitset(const boost::dynamic_bitset<>& pBits) const;          ///< Convert a bitset to IO register bytes.

private:
    bool initModule() override;
    //
    void resetImpl() override;
    //
    std::uint8_t getModuleSoftwareVersion() const override;
    std::uint8_t getModuleFirmwareVersion() override;
    //
    static std::map<std::string, RegisterDescr, std::less<>> getRegisterDescrs(const LayerConfig& pConfig);
                                                                                                    ///< \brief Generate the map of registers
                                                                                                    ///  depending on the configured bit count.

private:
    const std::uint64_t size;                                   ///< Number of IO bits.
    const std::uint32_t ioBytes;                                ///< Number of register bytes occupied by \ref size bits.

private:
    static constexpr std::uint8_t requireFirmwareVersion = 0;   ///< Compatible version of the controlled firmware module.

    CASIL_REGISTER_DRIVER_H("GPIO")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_GPIO_H
