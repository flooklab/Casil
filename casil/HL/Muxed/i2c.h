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

#ifndef CASIL_LAYERS_HL_I2C_H
#define CASIL_LAYERS_HL_I2C_H

#include <casil/HL/registerdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief %Driver for the \e i2c firmware module.
 *
 * Enables to make transactions through the Inter-Integrated Circuit (%I2C) master controller provided by
 * the firmware module in order to send/receive data to/from %I2C slave devices at a configurable address.
 *
 * Available registers:
 *
 * <table>
 * <tr><th>Name</th><th>\ref casil::HL::RegisterDescr::DataType "DataType"</th><th>\ref casil::HL::RegisterDescr::AccessMode "AccessMode"</th>
 * <th>Size</th><th>Description</th></tr>
 * <tr><td>\c RESET</td>    <td>Value</td>    <td>WriteOnly</td><td>8</td>          <td>Soft reset active on write to address.</td></tr>
 * <tr><td>\c VERSION</td>  <td>Value</td>    <td>ReadOnly</td> <td>8</td>          <td>Firmware module version.</td></tr>
 * <tr><td>\c START</td>    <td>Value</td>    <td>WriteOnly</td><td>8</td>          <td>Start transaction on write to address.</td></tr>
 * <tr><td>\c READY</td>    <td>Value</td>    <td>ReadOnly</td> <td>1</td>          <td>Transaction was completed.</td></tr>
 * <tr><td>\c NO_ACK</td>   <td>Value</td>    <td>ReadOnly</td> <td>1</td>          <td>Received \c NACK bit (not acknowledged).</td></tr>
 * <tr><td>\c ADDR</td>     <td>Value</td>    <td>ReadWrite</td><td>8</td>          <td>7-bit address of %I2C slave to talk to plus
 *                                                                                      read(1)/write(0) bit in LSB position.</td></tr>
 * <tr><td>\c SIZE</td>     <td>Value</td>    <td>ReadWrite</td><td>16</td>         <td>Transaction length in number of bytes
 *                                                                                      (must not exceed \c MEM_BYTES).</td></tr>
 * <tr><td>\c MEM_BYTES</td><td>Value</td>    <td>ReadOnly</td> <td>16</td>         <td>Size of transaction memory.</td></tr>
 * </table>
 */
class I2C final : public RegisterDriver
{
public:
    I2C(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);     ///< Constructor.
    ~I2C() override = default;                                                      ///< Default destructor.
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;    ///< Get the content of the transaction memory.
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override; ///< Set the content of the transaction memory.
    void exec() override;                                                                       ///< Start the transaction.
    bool isDone() override;                                                                     ///< Check if the transaction was completed.
    //
    void start();                                                                               ///< Start the transaction.
    bool isReady();                                                                             ///< Check if the transaction was completed.
    std::uint16_t memSize() const;                                                              ///< Get the \c MEM_BYTES register.
    std::uint8_t getAddr() const;                                                               ///< Get the \c ADDR register.
    void setAddr(std::uint8_t pAddr);                                                           ///< Set the \c ADDR register.
    std::bitset<7> getSlaveAddr() const;                                                        ///< Get slave address from \c ADDR register.
    void setSlaveAddr(const std::bitset<7>& pAddr);                                             ///< Set slave address for \c ADDR register.
    bool getReadWriteBit() const;                                                               ///< Get read/write bit from \c ADDR register.
    void setReadWriteBit(bool pRead);                                                           ///< Set read/write bit for \c ADDR register.
    std::uint16_t getSize() const;                                                              ///< Get the \c SIZE register.
    void setSize(int pSize);                                                                    ///< Set the \c SIZE register.
    std::vector<std::uint8_t> sendRead(std::uint8_t pAddr, int pSize = -1);                     ///< Configure and start a read transaction.
    std::vector<std::uint8_t> sendRead(const std::bitset<7>& pAddr, int pSize = -1);            ///< Configure and start a read transaction.
    void sendWrite(std::uint8_t pAddr, const std::vector<std::uint8_t>& pData);                 ///< Configure and start a write transaction.
    void sendWrite(const std::bitset<7>& pAddr, const std::vector<std::uint8_t>& pData);        ///< Configure and start a write transaction.

private:
    bool initModule() override;
    //
    void resetImpl() override;
    //
    std::uint8_t getModuleSoftwareVersion() const override;
    std::uint8_t getModuleFirmwareVersion() override;

private:
    std::uint32_t seqMemSize;                                   ///< Size of the firmware module instance's transaction memory.

private:
    static constexpr std::uint8_t requireFirmwareVersion = 1;   ///< Compatible version of the controlled firmware module.
    //
    static constexpr std::uint32_t seqMemOffs = 8;              ///< Offset of transaction memory with respect to the module base address.

    CASIL_REGISTER_DRIVER_H("I2C")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_I2C_H
