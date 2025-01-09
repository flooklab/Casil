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
*/

#include "invalidregdriver.h"

#include <utility>

using casil::HL::InvalidRegDriver;

CASIL_REGISTER_DRIVER_CPP(InvalidRegDriver)

//

InvalidRegDriver::InvalidRegDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, pConfig, LayerConfig(), getRegisterDescrs(pConfig))
{
}

//Private

void InvalidRegDriver::resetImpl()
{
}

//

std::uint8_t InvalidRegDriver::getModuleSoftwareVersion() const
{
    return 0;
}

std::uint8_t InvalidRegDriver::getModuleFirmwareVersion()
{
    return 0;
}

//

std::map<std::string, casil::HL::RegisterDescr, std::less<>> InvalidRegDriver::getRegisterDescrs(const LayerConfig& pConfig)
{
    const int failureMode = pConfig.getInt("failureMode", -1);

    switch (failureMode)
    {
        case 0:
            return {{"Version", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0}, .size{8},  .offs{0}}}};    //Name
        case 1:
            return {{"VERSION", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0}, .size{0},  .offs{0}}}};    //Size
        case 2:
            return {{"VERSION", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0}, .size{65}, .offs{0}}}};    //Size
        case 3:
            return {{"OUTPUT",  {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{4}, .size{3},  .offs{1}}}};    //Offset
        case 4:
        {
            return {{"VERSION", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0}, .size{8},  .offs{0},   //Default for R/O
                                 .defaultValue{std::uint64_t{0}}}}};
        }
        case 5:
        {
            return {{"FOOBAR",  {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{1}, .size{10}, .offs{3},   //Default type
                                 .defaultValue{std::vector<std::uint8_t>{0xDE, 0xBC}}}}};
        }
        case 6:
        {
            return {{"OUTPUT",  {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{4}, .size{3},  .offs{0},   //Default type
                                 .defaultValue{std::uint64_t{0}}}}};
        }
        case 7:
        {
            return {{"OUTPUT",  {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{4}, .size{3},  .offs{0},   //Default length
                                 .defaultValue{std::vector<std::uint8_t>{0xDE, 0xBC}}}}};
        }
        case -1:
        default:    //No failure
        {
            return {{"VERSION", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0}, .size{8},  .offs{0}}},
                    {"FOOBAR",  {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{1}, .size{10}, .offs{3},
                                 .defaultValue{std::uint64_t{0}}}},
                    {"OUTPUT",  {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{4}, .size{3},  .offs{0},
                                 .defaultValue{std::vector<std::uint8_t>{0xDE, 0xBC, 0x9A}}}}};
        }
    }
}
