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

#include <casil/HL/Direct/scpi.h>

#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/env.h>
#include <casil/logger.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <set>
#include <stdexcept>

using casil::HL::SCPI;

CASIL_REGISTER_DRIVER_CPP(SCPI)
CASIL_REGISTER_DRIVER_ALIAS("scpi")

//

SCPI::SCPI(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig::fromYAML(
                     "{init: {device: string}}")
                 ),
    deviceDescription(Auxil::propertyTreeFromYAML(loadDeviceDescription(config.getStr("init.device")))),
    writeCommands(parseWriteCommands(deviceDescription)),
    queryCommands(parseQueryCommands(deviceDescription)),
    deviceIdentifier(parseDeviceIdentifier(deviceDescription))
{
}

//Public

std::optional<std::string> SCPI::operator()(const std::string_view pCmd, const std::optional<int> pChannel, VariantValueType pValue) const
{
    return command(pCmd, pChannel, std::move(pValue));
}

//

void SCPI::writeCommand(const std::string_view pCmd, const std::optional<int> pChannel, VariantValueType pValue) const
{
    const int channel = (pChannel.has_value() ? *pChannel : -1);

    if (std::holds_alternative<std::monostate>(pValue))
    {
        if (isSetter(pCmd))
        {
            throw std::invalid_argument("The SCPI command \"" + std::string(pCmd) + "\" is a setter and needs a value argument "
                                        "(driver: \"" + name + "\").");
        }

        interface.write(getWriteCommand(pCmd, channel));
    }
    else
    {
        if (!isSetter(pCmd))
        {
            Logger::logWarning("Dropping value argument for " + getSelfDescription() + " because \"" + std::string(pCmd) + "\" " +
                               "is not a setter.");
        }

        std::vector<std::uint8_t> cmd = getWriteCommand(pCmd, channel);

        std::vector<std::uint8_t> cmdSuffix;

        if (std::holds_alternative<std::string>(pValue))
            cmdSuffix = Bytes::byteVecFromStr(" " + std::move(std::get<std::string>(pValue)));
        else
            cmdSuffix = Bytes::byteVecFromStr(" " + getValueStr(std::move(pValue)));

        cmd.insert(cmd.end(), cmdSuffix.begin(), cmdSuffix.end());

        interface.write(cmd);
    }
}

std::string SCPI::queryCommand(const std::string_view pCmd, const std::optional<int> pChannel) const
{
    const int channel = (pChannel.has_value() ? *pChannel : -1);

    return Bytes::strFromByteVec(interface.query(getQueryCommand(pCmd, channel)));
}

std::optional<std::string> SCPI::command(const std::string_view pCmd, const std::optional<int> pChannel, VariantValueType pValue) const
{
    const int channel = (pChannel.has_value() ? *pChannel : -1);

    if (isQueryCommand(pCmd, channel))
    {
        if (!std::holds_alternative<std::monostate>(pValue))
        {
            Logger::logWarning("Dropping value argument for " + getSelfDescription() + " because \"" + std::string(pCmd) + "\" " +
                               "is a query command.");
        }

        return queryCommand(pCmd, channel);
    }
    else
    {
        writeCommand(pCmd, channel, std::move(pValue));
        return std::nullopt;
    }
}

//Private

bool SCPI::initImpl()
{
    std::string ident;
    try
    {
        ident = queryCommand("get_name");
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not initialize " + getSelfDescription() + ": " + exc.what());
        return false;
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Could not initialize SCPI driver \"" + name + "\": " +
                                 "No \"get_name\" query command. THIS SHOULD NEVER HAPPEN!");
    }

    if (ident != deviceIdentifier)
    {
        Logger::logError("Wrong SCPI device description configured for " + getSelfDescription() +
                         " (expected identifier: \"" + deviceIdentifier + "\"; actual identifier: \"" + ident + "\").");
        return false;
    }

    return true;
}

bool SCPI::closeImpl()
{
    return true;
}

//

bool SCPI::isQueryCommand(const std::string_view pCmd, const int pChannel) const
{
    const CommandMapType& queryCmds = getQueryCommandMap(pChannel);

    return queryCmds.contains(pCmd);
}

const std::vector<std::uint8_t>& SCPI::getWriteCommand(const std::string_view pCmd, const int pChannel) const
{
    const CommandMapType& writeCmds = getWriteCommandMap(pChannel);

    const auto it = writeCmds.find(pCmd);

    if (it == writeCmds.end())
        throw std::invalid_argument("The command \"" + std::string(pCmd) + "\" is not available for SCPI driver \"" + name + "\".");

    return it->second;
}

const std::vector<std::uint8_t>& SCPI::getQueryCommand(const std::string_view pCmd, const int pChannel) const
{
    const CommandMapType& queryCmds = getQueryCommandMap(pChannel);

    const auto it = queryCmds.find(pCmd);

    if (it == queryCmds.end())
        throw std::invalid_argument("The command \"" + std::string(pCmd) + "\" is not available for SCPI driver \"" + name + "\".");

    return it->second;
}

//

const SCPI::CommandMapType& SCPI::getWriteCommandMap(const int pChannel) const
{
    const auto it = writeCommands.find(pChannel);

    if (it == writeCommands.end())
        throw std::invalid_argument("Channel number " + std::to_string(pChannel) + " is not available for SCPI driver \"" + name + "\".");

    return it->second;
}

const SCPI::CommandMapType& SCPI::getQueryCommandMap(const int pChannel) const
{
    const auto it = queryCommands.find(pChannel);

    if (it == queryCommands.end())
        throw std::invalid_argument("Channel number " + std::to_string(pChannel) + " is not available for SCPI driver \"" + name + "\".");

    return it->second;
}

//

std::string SCPI::loadDeviceDescription(const std::string& pDeviceType)
{
    std::string fileName = pDeviceType;

    //Obtain actual file name by transforming device type argument to lowercase, replacing spaces with underscores and appending suffix
    std::transform(fileName.begin(), fileName.end(), fileName.begin(),
                   [](const unsigned char pChar){ return std::tolower(pChar); });
    std::replace(fileName.begin(), fileName.end(), ' ', '_');
    fileName += ".yaml";

    const std::set<std::string>& envDirs = Env::getEnv("CASIL_DEV_DESC_DIRS");

    std::filesystem::path filePath;

    for (std::filesystem::path tmpPath : envDirs)
    {
        if (!std::filesystem::exists(tmpPath) && !filePath.empty()) //Ignore this skip for still empty 'filePath' in order to have an
            continue;                                               //actual file name to print in case that none of the paths exists

        tmpPath /= fileName;

        if (!std::filesystem::exists(tmpPath) && !filePath.empty()) //Ignore this skip for still empty 'filePath', see comment above
            continue;

        filePath = std::move(tmpPath);
    }

    if (!std::filesystem::exists(filePath))
    {
        throw std::runtime_error("Could not find SCPI device description file \"" + filePath.string() +
                                 "\" (requested device type: \"" + pDeviceType + "\").");
    }

    std::string devDescStr;

    try
    {
        std::ifstream devDescFile;
        devDescFile.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        devDescFile.open(filePath);

        try
        {
            for (std::string line; std::getline(devDescFile, line, '\n');)
                devDescStr.append(line).append("\n");
        }
        catch (const std::ios_base::failure&)
        {
            if (!devDescFile.eof())
                throw;
        }

        devDescFile.close();
    }
    catch (const std::ios_base::failure&)
    {
        throw std::runtime_error("Could not load SCPI device description file \"" + filePath.string() +
                                 "\" (requested device type: \"" + pDeviceType + "\").");
    }

    return devDescStr;
}

std::map<int, SCPI::CommandMapType> SCPI::parseWriteCommands(const boost::property_tree::ptree& pDeviceDescription)
{
    //SCPI write commands mandatory by IEEE 488.2 standard
    static const std::vector<std::pair<std::string, std::string>> ieeeCmds = {std::make_pair("clear", "*CLS"),
                                                                              std::make_pair("reset", "*RST"),
                                                                              std::make_pair("trigger", "*TRG")};

    return parseCommands(pDeviceDescription, false, ieeeCmds);
}

std::map<int, SCPI::CommandMapType> SCPI::parseQueryCommands(const boost::property_tree::ptree& pDeviceDescription)
{
    //SCPI query commands mandatory by IEEE 488.2 standard
    static const std::vector<std::pair<std::string, std::string>> ieeeCmds = {std::make_pair("get_name", "*IDN?")};

    return parseCommands(pDeviceDescription, true, ieeeCmds);
}

std::map<int, SCPI::CommandMapType> SCPI::parseCommands(const boost::property_tree::ptree& pDeviceDescription, bool pQueryCommands,
                                                        const std::vector<std::pair<std::string, std::string>>& pIEEECmds)
{
    //Either only use all write commands or all query commands; also throw exception if empty
    auto skipCmd = [pQueryCommands](const std::string& pCmdKey, const std::string& pCmdVal) -> bool
    {
        if (pCmdVal.empty())
            throw std::runtime_error("Command \"" + pCmdKey + "\" from SCPI device description is empty.");

        if (pQueryCommands) //Skip write commands
        {
            if (pCmdVal.back() != '?')
                return true;
        }
        else                //Skip query commands
        {
            if (pCmdVal.back() == '?')
                return true;
        }

        return false;
    };

    static const std::string channelPrefix = "channel ";

    CommandMapType cmdsTopLevel;

    //SCPI commands mandatory by IEEE 488.2 standard
    for (const auto& ieeeCmd : pIEEECmds)
        cmdsTopLevel.insert(std::make_pair(ieeeCmd.first, Bytes::byteVecFromStr(ieeeCmd.second)));

    std::vector<std::string> channels;

    for (const auto& [key, val] : pDeviceDescription)
    {
        if (key == "identifier")
            continue;

        if (key.starts_with(channelPrefix))
        {
            channels.push_back(key);
            continue;
        }

        if (skipCmd(key, val.data()))
            continue;

        cmdsTopLevel.insert(std::make_pair(key, Bytes::byteVecFromStr(val.data())));
    }

    std::map<int, SCPI::CommandMapType> tCommands;

    tCommands.insert(std::make_pair(-1, std::move(cmdsTopLevel)));

    for (const auto& channel : channels)
    {
        int channelNr = 0;

        try
        {
            channelNr = std::stoul(channel.substr(channelPrefix.length()));

            if (channelNr < 0)
                throw std::out_of_range("");
        }
        catch (const std::invalid_argument&)
        {
            throw std::runtime_error("Could not parse channel number in SCPI device description.");
        }
        catch (const std::out_of_range&)
        {
            throw std::runtime_error("Parsed channel number from SCPI device description is out of range.");
        }

        CommandMapType cmdsCurrentChannel;

        const boost::property_tree::ptree& channelTree = pDeviceDescription.find(channel)->second;

        for (const auto& [key, val] : channelTree)
        {
            if (val.data().empty())
                throw std::runtime_error("Command \"" + key + "\" from SCPI device description is empty.");

            if (skipCmd(key, val.data()))
                continue;

            cmdsCurrentChannel.insert(std::make_pair(key, Bytes::byteVecFromStr(val.data())));
        }

        tCommands.insert(std::make_pair(channelNr, std::move(cmdsCurrentChannel)));
    }

    return tCommands;
}

std::string SCPI::parseDeviceIdentifier(const boost::property_tree::ptree& pDeviceDescription)
{
    const auto it = pDeviceDescription.find("identifier");

    if (it == pDeviceDescription.not_found())
        throw std::runtime_error("Could not find device identifier in SCPI device description.");

    return it->second.data();
}

//

std::string SCPI::getValueStr(VariantValueType&& pValue)
{
    if (std::holds_alternative<std::string>(pValue))
        return std::move(std::get<std::string>(pValue));
    else if (std::holds_alternative<double>(pValue))
        return std::format("{: <-#E}", std::get<double>(pValue));
    else if (std::holds_alternative<int>(pValue))
        return std::format("{: <-d}", std::get<int>(pValue));
    else
        return "";  //Fallback for std::monostate
}

bool SCPI::isSetter(const std::string_view pCmd)
{
    return pCmd.starts_with("set_");
}
