/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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
*/

#include "testregdriver.h"

#include <utility>

using casil::HL::TestRegDriver;

CASIL_REGISTER_DRIVER_CPP(TestRegDriver)

//

TestRegDriver::TestRegDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, pConfig, LayerConfig(),
       {{"RESET",     {.type{DataType::Value},     .mode{AccessMode::WriteOnly}, .addr{0},  .size{8},  .offs{0}}},
        {"VERSION",   {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0},  .size{8},  .offs{0}}},
        {"FOOBAR",    {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{1},  .size{10}, .offs{3}}},
        {"INPUT",     {.type{DataType::ByteArray}, .mode{AccessMode::ReadOnly},  .addr{3},  .size{1},  .offs{0}}},
        {"OUTPUT",    {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{4},  .size{3},  .offs{0}}},
        {"TRIGGER",   {.type{DataType::ByteArray}, .mode{AccessMode::WriteOnly}, .addr{7},  .size{2},  .offs{0}}},
        {"TESTARR",   {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{9},  .size{2},  .offs{0},
                                                                                 .defaultValue{std::vector<std::uint8_t>{0xDE, 0xBC}}}},
        {"TESTVAL",   {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{11}, .size{16}, .offs{0}, .defaultValue{0xABCDu}}},
        //
        {"TESTVAL_A", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{13}, .size{18}, .offs{3}}},
        {"TESTVAL_B", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{16}, .size{11}, .offs{5}}},
        {"TESTVAL_C", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{18}, .size{11}, .offs{0}}},
        //
        {"TESTVAL_D", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{20}, .size{26}, .offs{3}}},
        {"TESTVAL_E", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{24}, .size{34}, .offs{3}}},
        {"TESTVAL_F", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{29}, .size{58}, .offs{3}}},
        {"TESTVAL_G", {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{37}, .size{63}, .offs{3}}}}),
    initCount(0),
    closeCount(0)
{
}

//Private

bool TestRegDriver::initModule()
{
    if (initCount++ >= 2)
        return false;

    return true;
}

bool TestRegDriver::closeModule()
{
    if (closeCount++ >= 2)
        return false;

    return true;
}

//

void TestRegDriver::resetImpl()
{
    setValue("RESET", 0);
}

//

std::uint8_t TestRegDriver::getModuleSoftwareVersion() const
{
    return requireFirmwareVersion;
}

std::uint8_t TestRegDriver::getModuleFirmwareVersion()
{
    return static_cast<std::uint8_t>(getValue("VERSION"));
}
