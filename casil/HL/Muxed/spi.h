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

#ifndef CASIL_LAYERS_HL_SPI_H
#define CASIL_LAYERS_HL_SPI_H

#include <casil/HL/registerdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief %Driver for the \e spi firmware module.
 *
 * Enables to make transactions through the Serial Peripheral Interface (%SPI) master controller
 * provided by the firmware module in order to send/receive data to/from %SPI slave devices.
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
 * <tr><td>\c SIZE</td>     <td>Value</td>    <td>ReadWrite</td><td>16</td>         <td>Transaction length in number of \e bits
 *                                                                                      (must not exceed <tt>MEM_BYTES * 8</tt>).</td></tr>
 * <tr><td>\c WAIT</td>     <td>Value</td>    <td>ReadWrite</td><td>32</td>     <td>Time delay between repetitions in clock cycles.</td></tr>
 * <tr><td>\c REPEAT</td>   <td>Value</td>    <td>ReadWrite</td><td>32</td>         <td>Number of repetitions of the %SPI transaction
 *                                                                                      (zero for infinite repetitions).</td></tr>
 * <tr><td>\c EN</td>       <td>Value</td>    <td>ReadWrite</td><td>1</td>          <td>Enable automatic transaction start on rising
 *                                                                                      \c EXT_START signal (on FPGA).</td></tr>
 * <tr><td>\c MEM_BYTES</td><td>Value</td>    <td>ReadOnly</td> <td>16</td>         <td>Size (in bytes) of the transaction memory for the
 *                                                                                      transmit sequence as well as size of the transaction
 *                                                                                      memory for the readback sequence.</td></tr>
 * </table>
 */
class SPI final : public RegisterDriver
{
public:
    SPI(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);     ///< Constructor.
    ~SPI() override = default;                                                      ///< Default destructor.
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;    ///< Get the content of the transaction memory.
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override; ///< Set the content of the transaction memory.
    void exec() override;                                                                       ///< Start the transaction.
    bool isDone() override;                                                                     ///< Check if the transaction was completed.
    //
    void start();                                                                               ///< Start the transaction.
    bool isReady();                                                                             ///< Check if the transaction was completed.
    std::uint16_t memSize() const;                                                              ///< Get the \c MEM_BYTES register.
    std::uint16_t getSize() const;                                                              ///< Get the \c SIZE register.
    void setSize(int pSize);                                                                    ///< Set the \c SIZE register.
    std::uint32_t getWait() const;                                                              ///< Get the \c WAIT register.
    void setWait(std::uint32_t pCycles);                                                        ///< Set the \c WAIT register.
    std::uint32_t getRepeat() const;                                                            ///< Get the \c REPEAT register.
    void setRepeat(std::uint32_t pReps);                                                        ///< Set the \c REPEAT register.
    bool getEn() const;                                                                         ///< Get the \c EN register.
    void setEn(bool pEnable);                                                                   ///< Set the \c EN register.
    void sendCmd(const std::vector<std::uint8_t>& pCmd);                                        ///< Configure and start a transaction.
    void sendCmd(std::uint8_t pCmd);                                                            ///< Configure and start a transaction.
    void sendCmd(std::uint8_t pCmdMsb, std::uint8_t pCmdLsb);                                   ///< Configure and start a transaction.
    std::vector<std::uint8_t> sendCmdRead(const std::vector<std::uint8_t>& pCmd, int pSize = -1);   ///< \brief Configure and start a
                                                                                                    ///  transaction and get the readback.
    std::vector<std::uint8_t> sendCmdRead(std::uint8_t pCmd);                                   ///< \brief Configure and start a
                                                                                                ///  transaction and get the readback.
    std::vector<std::uint8_t> sendCmdRead(std::uint8_t pCmdMsb, std::uint8_t pCmdLsb);          ///< \brief Configure and start a
                                                                                                ///  transaction and get the readback.

private:
    bool initModule() override;
    //
    void resetImpl() override;
    //
    std::uint8_t getModuleSoftwareVersion() const override;
    std::uint8_t getModuleFirmwareVersion() override;

private:
    std::uint32_t seqMemSize;                                   ///< Size of the firmware module instance's transaction memory in bytes.

private:
    static constexpr std::uint8_t requireFirmwareVersion = 2;   ///< Compatible version of the controlled firmware module.
    //
    static constexpr std::uint32_t seqMemOffs = 16;             ///< Offset of transaction memory with respect to the module base address.

    CASIL_REGISTER_DRIVER_H("SPI")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_SPI_H
