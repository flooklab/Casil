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
*/

#ifndef CASILTESTS_TESTBACKENDDRIVER_H
#define CASILTESTS_TESTBACKENDDRIVER_H

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

class TestBackendDriver final : public MuxedDriver
{
public:
    TestBackendDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);
    ~TestBackendDriver() override = default;
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override;
    void exec() override;
    bool isDone() override;
    //
    std::uint64_t getSomeSpecialValue() const;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    std::vector<std::uint8_t> inData;
    std::vector<std::uint8_t> outData;

    CASIL_REGISTER_DRIVER_H("TestBackendDriver")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASILTESTS_TESTBACKENDDRIVER_H
