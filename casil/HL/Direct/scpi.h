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

#ifndef CASIL_HL_SCPI_H
#define CASIL_HL_SCPI_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace casil
{

namespace HL
{

/*!
 * \brief %Driver for \e Standard \e Commands \e for \e Programmable \e Instruments (%SCPI) devices.
 *
 * \todo Detailed doc
 */
class SCPI final : public DirectDriver
{
public:
    typedef std::variant<std::monostate, std::string, int, double> VariantValueType;        ///< Supported command argument types.

private:
    typedef std::map<std::string, std::vector<std::uint8_t>, std::less<>> CommandMapType;   ///< \brief Map type for commands (as byte sequence)
                                                                                            ///  with the command names as keys.

public:
    SCPI(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);            ///< Constructor.
    ~SCPI() override = default;                                                             ///< Default destructor.
    //
    std::optional<std::string> operator()(std::string_view pCmd, std::optional<int> pChannel = std::nullopt,
                                          VariantValueType pValue = std::monostate{}) const;    ///< Execute a command (either write or query).
    //
    void writeCommand(std::string_view pCmd, std::optional<int> pChannel = std::nullopt, VariantValueType pValue = std::monostate{}) const;
                                                                                                            ///< Execute a write command.
    std::string queryCommand(std::string_view pCmd, std::optional<int> pChannel = std::nullopt) const;      ///< Execute a query command.
    std::optional<std::string> command(std::string_view pCmd, std::optional<int> pChannel = std::nullopt,
                                       VariantValueType pValue = std::monostate{}) const;       ///< Execute a command (either write or query).

private:
    bool initImpl() override;
    bool closeImpl() override;
    //
    bool isQueryCommand(std::string_view pCmd, int pChannel = -1) const;                                ///< Check if command is query command.
    const std::vector<std::uint8_t>& getWriteCommand(std::string_view pCmd, int pChannel = -1) const;   ///< Get a write command from its name.
    const std::vector<std::uint8_t>& getQueryCommand(std::string_view pCmd, int pChannel = -1) const;   ///< Get a query command from its name.
    //
    const CommandMapType& getWriteCommandMap(int pChannel = -1) const;                      ///< Get the write commands for a certain channel.
    const CommandMapType& getQueryCommandMap(int pChannel = -1) const;                      ///< Get the query commands for a certain channel.
    //
    static std::string loadDeviceDescription(const std::string& pDeviceType);       ///< Read the device description file for some device type.
    static std::map<int, CommandMapType> parseWriteCommands(const boost::property_tree::ptree& pDeviceDescription);
                                                                                    ///< \brief Generate a map of general and channel-specific
                                                                                    ///  write commands from a device description.
    static std::map<int, CommandMapType> parseQueryCommands(const boost::property_tree::ptree& pDeviceDescription);
                                                                                    ///< \brief Generate a map of general and channel-specific
                                                                                    ///  query commands from a device description.
    static std::map<int, CommandMapType> parseCommands(const boost::property_tree::ptree& pDeviceDescription, bool pQueryCommands,
                                                       const std::vector<std::pair<std::string, std::string>>& pIEEECmds);
                                                                                    ///< \brief Generate a map of general and
                                                                                    ///  channel-specific commands from a device
                                                                                    ///  description (either query or write commands).
    static std::string parseDeviceIdentifier(const boost::property_tree::ptree& pDeviceDescription);    ///< Get the device identifier string.
    //
    static std::string getValueStr(VariantValueType&& pValue);                      ///< Convert a value to string with fixed formatting.
    static bool isSetter(std::string_view pCmd);                                    ///< Check if a command name identifies it as setter.

private:
    const boost::property_tree::ptree deviceDescription;    ///< YAML tree of device description file (general and channel-specific commands).
    const std::map<int, CommandMapType> writeCommands;      ///< Write commands implemented by the device with channel as key (-1: no channel).
    const std::map<int, CommandMapType> queryCommands;      ///< Query commands implemented by the device with channel as key (-1: no channel).
    //
    const std::string deviceIdentifier;                     ///< Device identifier string returned by the "*IDN?" command.

    CASIL_REGISTER_DRIVER_H("SCPI")
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_SCPI_H
