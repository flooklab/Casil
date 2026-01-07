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

#include "testbackenddriver.h"

#include <algorithm>
#include <iterator>
#include <utility>

using casil::Layers::HL::TestBackendDriver;

CASIL_REGISTER_DRIVER_CPP(TestBackendDriver)

//

TestBackendDriver::TestBackendDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    MuxedDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig()),
    inData(),
    outData()
{
}

//Public

std::vector<std::uint8_t> TestBackendDriver::getData(int, std::uint32_t)
{
    return outData;
}

void TestBackendDriver::setData(const std::vector<std::uint8_t>& pData, std::uint32_t)
{
    inData = pData;
}

void TestBackendDriver::exec()
{
    outData.clear();
    std::ranges::reverse_copy(inData.begin(), inData.end(), std::back_inserter(outData));
}

bool TestBackendDriver::isDone()
{
    return true;
}

//

std::uint64_t TestBackendDriver::getSomeSpecialValue() const
{
    return 0x1B97C5A3DF2C6B4Au;
}

//Private

bool TestBackendDriver::initImpl()
{
    return true;
}

bool TestBackendDriver::closeImpl()
{
    return true;
}
