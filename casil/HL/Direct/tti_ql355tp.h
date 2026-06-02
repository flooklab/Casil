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

#ifndef CASIL_LAYERS_HL_TTI_QL355TP_H
#define CASIL_LAYERS_HL_TTI_QL355TP_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <string>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief %Driver for the \e Aim-TTi \e QL355TP power supply.
 *
 * Controls the channel/voltage/current settings of the Aim-TTi QL355TP power supply via %SCPI commands
 * and, likewise, lets you measure voltage/current values of the outputs. You may instead want to use the
 * SCPI driver with the corresponding \e device \e description \e file for the QL355TP instead, which is more
 * complete than this driver (in terms of implemented commands). But this driver at least provides built-in conversion
 * for the measurement readbacks to values in floating-point format, which the generalized SCPI driver does not have.
 *
 * Note: The %SCPI device identifier is initially checked during init() using getDeviceIdentifier()
 *       and init() fails (returning false) if that does not equal the expected "QL355TP".
 */
class TTiQL355TP final : public DirectDriver
{
public:
    TTiQL355TP(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);  ///< Constructor.
    ~TTiQL355TP() override = default;                                                   ///< Default destructor.
    //
    bool reInitIntf() const;                                                            ///< Re-initialize underlying interface instance.
    //
    void writeCmd(const std::string& pCmd) const;                                       ///< Execute an arbitrary write command.
    std::string queryCmd(const std::string& pCmd) const;                                ///< Execute an arbitrary query command.
    //
    std::string getDeviceIdentifier() const;                                            ///< Get the %SCPI device identifier.
    //
    void setEnable(bool pOn, int pChannel) const;                                       ///< Enable/disable one or all output channels.
    double getVoltage(int pChannel) const;                                              ///< Get the measured output voltage of a channel.
    double getSetVoltage(int pChannel) const;                                           ///< Get the set output voltage of a channel.
    void setVoltage(double pValue, int pChannel) const;                                 ///< Set the output voltage of a channel.
    double getCurrent(int pChannel) const;                                              ///< Get the measured output current of a channel.
    double getCurrentLimit(int pChannel) const;                                         ///< Get the set current limit of a channel.
    void setCurrentLimit(double pValue, int pChannel) const;                            ///< Set the current limit of a channel.
    //
    void resetTrip() const;                                                             ///< Reset the OVP and OCP trips.

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_DRIVER_H("TTiQL355TP")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_TTI_QL355TP_H
