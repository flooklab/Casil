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

#ifndef CASIL_HL_DIRECTDRIVER_H
#define CASIL_HL_DIRECTDRIVER_H

#include <casil/HL/driver.h>

#include <casil/layerconfig.h>
#include <casil/TL/directinterface.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace HL
{

class DirectDriver : public Driver
{
public:
    typedef TL::DirectInterface InterfaceBaseType;

public:
    DirectDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                 LayerConfig pConfig, const LayerConfig& pRequiredConfig);
    ~DirectDriver() override = default;

private:
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override final;
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override final;
    void exec() override final;
    bool isDone() override final;
    //
    bool initImpl() override = 0;
    bool closeImpl() override = 0;

protected:
    InterfaceBaseType& interface;
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_DIRECTDRIVER_H
