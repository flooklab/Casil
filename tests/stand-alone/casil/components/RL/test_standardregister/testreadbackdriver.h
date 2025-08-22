/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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

#ifndef CASILTESTS_TESTREADBACKDRIVER_H
#define CASILTESTS_TESTREADBACKDRIVER_H

#include <casil/HL/muxeddriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::HL
{

class TestReadbackDriver final : public MuxedDriver
{
public:
    TestReadbackDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);
    ~TestReadbackDriver() override = default;
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override;
    void exec() override;
    bool isDone() override;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    std::vector<std::uint8_t> data;
    bool executed;
    bool failGetData;

    CASIL_REGISTER_DRIVER_H("TestReadbackDriver")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASILTESTS_TESTREADBACKDRIVER_H
