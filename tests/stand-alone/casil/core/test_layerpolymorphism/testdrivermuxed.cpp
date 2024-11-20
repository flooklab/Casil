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

#include "testdrivermuxed.h"

#include <utility>
#include <stdexcept>

using casil::TestDriverMuxed;

CASIL_REGISTER_DRIVER_CPP(TestDriverMuxed)

//

TestDriverMuxed::TestDriverMuxed(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    MuxedDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//

std::vector<std::uint8_t> TestDriverMuxed::getData(int, std::uint32_t)
{
    return {1, 1, 2, 3, 5, 8, 13};
}

void TestDriverMuxed::setData(const std::vector<std::uint8_t>& pData, std::uint32_t)
{
    if (pData == std::vector<std::uint8_t>{1, 2, 3, 4, 5})
        throw std::runtime_error("This exception is a test...");
}

//

bool TestDriverMuxed::someSpecialFunctionality(int pParam) const
{
    if (pParam == 123)
        return true;
    return false;
}

//Private

bool TestDriverMuxed::initImpl()
{
    return true;
}

bool TestDriverMuxed::closeImpl()
{
    return true;
}
