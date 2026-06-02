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

#ifndef CASIL_LAYERS_HL_SI570_H
#define CASIL_LAYERS_HL_SI570_H

#include <casil/HL/metadriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/HL/Muxed/i2c.h>
#include <casil/RL/standardregister.h>

#include <cstdint>
#include <string>
#include <tuple>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief Meta driver for the %Si570 clock generator IC that is interfaced via an \e i2c firmware module.
 *
 * Controls the %Si570 clock generator through the backend driver via %I2C on address 93.
 * In particular, the device can be reset (see reset()) and its frequency can be configured (see changeFrequency()).
 * The frequency is initially set (during init()) to the frequency setting from the component configuration (see Si570()).
 *
 * Note: The configured backend driver \e must be of type I2C, as this is the used control interface of the %Si570.
 */
class Si570 final : public MetaDriver
{
public:
    Si570(std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig);         ///< Constructor.
    ~Si570() override = default;                                                        ///< Default destructor.
    //
    void reset() override;                                                              ///< Reset the device.
    //
    void changeFrequency(double pFreqMHz);                                              ///< Change reference frequency of %Si570 device.

private:
    std::tuple<std::uint8_t, std::uint8_t, std::uint64_t> readRegisters();              ///< Read relevant register values from %Si570 device.
    void modifyRegisters(std::uint8_t pHSDiv, std::uint8_t pN1, std::uint64_t pRFreq);  ///< Write relevant register values to %Si570 device.

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    I2C& i2cDrv;                                ///< The \ref MetaDriver::backendDriver "backendDriver" instance casted to needed I2C type.
    //
    RL::StandardRegister reg;                   ///< Internal register instance to facilitate %Si570 register handling.
    //
    const double initFrequency;                 ///< Initial reference frequency to set during init() in MHz.

    CASIL_REGISTER_META_DRIVER_H("Si570")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_SI570_H
