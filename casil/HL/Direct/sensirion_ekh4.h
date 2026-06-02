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

#ifndef CASIL_LAYERS_HL_SENSIRION_EKH4_H
#define CASIL_LAYERS_HL_SENSIRION_EKH4_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief %Driver for the \e Sensirion \e EK-H4 temperature/humidity sensor multiplexer box.
 *
 * This can be used to read up to four channels of Sensirion sensors for temperature and humidity. The dew point can be obtained as well.
 *
 * \note The device uses a type-length-value (TLV) protocol (via serial port at a baud rate of 115200).
 *       Apparently, the type byte definitions could not be found online and also the data returned by the device
 *       is often too long. Overall, the protocol is not well understood, but some conditions can be detected and
 *       fixed/interpreted by this driver accordingly. Nevertheless, it can happen that data of some channels cannot
 *       be reconstructed, in which case the returned results for those channels will simply be \c std::nullopt.
 *
 * \internal \note See also getValues() for more notes on that protocol and how it is parsed. \endinternal
 */
class SensirionEKH4 final : public DirectDriver
{
public:
    SensirionEKH4(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);   ///< Constructor.
    ~SensirionEKH4() override = default;                                                    ///< Default destructor.
    //
    std::array<std::optional<double>, 4> getTemperatures() const;                   ///< Get the temperature values from all four sensors.
    std::array<std::optional<double>, 4> getHumidities() const;                     ///< Get the humidity values from all four sensors.
    std::array<std::optional<double>, 4> getDewPoints() const;                      ///< Get the dew point values from all four sensors.
    //
    std::optional<double> getTemperature(int pChannel) const;                       ///< Get the temperature value of one of the sensors.
    std::optional<double> getHumidity(int pChannel) const;                          ///< Get the humidity value of one of the sensors.
    std::optional<double> getDewPoint(int pChannel) const;                          ///< Get the dew point value of one of the sensors.

private:
    void write(std::string_view pCmd) const;                                        ///< Write a command to the device.
    std::vector<std::uint8_t> query(std::string_view pCmd) const;                   ///< Query data from the device.
    //
    std::array<std::optional<double>, 4> getValues(std::string_view pCmd) const;    ///< \brief Query data for all four sensors and
                                                                                    ///  interpret them as individual measurement values.

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_DRIVER_H("SensirionEKH4")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_SENSIRION_EKH4_H
