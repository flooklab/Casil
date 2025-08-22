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

#include "testreadbackdriver.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

using casil::Layers::HL::TestReadbackDriver;

CASIL_REGISTER_DRIVER_CPP(TestReadbackDriver)

//

TestReadbackDriver::TestReadbackDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    MuxedDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig()),
    data(),
    executed(false),
    failGetData(false)
{
}

//Public

std::vector<std::uint8_t> TestReadbackDriver::getData(const int pSize, std::uint32_t)
{
    if (failGetData)
        return {};

    if (pSize < 0)
        throw std::invalid_argument("Invalid size argument.");

    const std::size_t tSize = static_cast<std::size_t>(pSize);

    if (tSize > data.size())
        throw std::invalid_argument("Invalid size argument.");

    return std::vector<std::uint8_t>(data.begin(), data.begin()+tSize);
}

void TestReadbackDriver::setData(const std::vector<std::uint8_t>& pData, std::uint32_t)
{
    if (pData == std::vector<std::uint8_t>{0b10101010u, 0b10101010u})   //Trigger unexpected behavior of getData() for test
        failGetData = true;

    data = pData;
}

void TestReadbackDriver::exec()
{
    data = {0xFFu, 0xFFu};
    executed = true;
}

bool TestReadbackDriver::isDone()
{
    return executed;
}

//Private

bool TestReadbackDriver::initImpl()
{
    return true;
}

bool TestReadbackDriver::closeImpl()
{
    return true;
}
