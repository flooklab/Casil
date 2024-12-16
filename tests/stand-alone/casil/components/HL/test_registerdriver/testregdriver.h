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
*/

#ifndef CASILTESTS_TESTREGDRIVER_H
#define CASILTESTS_TESTREGDRIVER_H

#include <casil/HL/registerdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>

namespace casil
{

namespace Layers::HL
{

class TestRegDriver final : public RegisterDriver
{
public:
    TestRegDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);
    ~TestRegDriver() override = default;

private:
    bool initModule() override;
    bool closeModule() override;
    //
    void resetImpl() override;
    //
    std::uint8_t getModuleSoftwareVersion() const override;
    std::uint8_t getModuleFirmwareVersion() override;

private:
    int initCount;
    int closeCount;

private:
    static constexpr std::uint8_t requireFirmwareVersion = 11;

    CASIL_REGISTER_DRIVER_H("TestRegDriver")
};

} // namespace HL

} // namespace casil

#endif // CASILTESTS_TESTREGDRIVER_H
