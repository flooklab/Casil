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

#ifndef CASIL_HL_DRIVER_H
#define CASIL_HL_DRIVER_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace HL
{

class Driver : public LayerBase
{
public:
    Driver(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
    ~Driver() override = default;
    //
    virtual void reset();
    //
    virtual std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) = 0;
    virtual void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) = 0;
    virtual void exec() = 0;
    virtual bool isDone() = 0;
    //
    //TODO could something be added here to enable direct C++ FunctionalRegister-functionality without python wrapper in between?

private:
    bool initImpl() override = 0;
    bool closeImpl() override = 0;
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_DRIVER_H
