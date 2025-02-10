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

#include "testinterface.h"

#include <utility>

using casil::TestInterface;

CASIL_REGISTER_INTERFACE_CPP(TestInterface)

//

TestInterface::TestInterface(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig())
{
}

//Public

std::vector<std::uint8_t> TestInterface::read(const std::uint64_t pAddr, const int pSize)
{
    (void)pAddr;
    (void)pSize;
    return {};
}

void TestInterface::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    (void)pAddr;
    (void)pData;
}

std::vector<std::uint8_t> TestInterface::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                               const std::vector<std::uint8_t>& pData, const int pSize)
{
    (void)pWriteAddr;
    (void)pReadAddr;
    (void)pData;
    (void)pSize;
    return {};
}

//

bool TestInterface::readBufferEmpty() const
{
    return true;
}

void TestInterface::clearReadBuffer()
{
}

//Private

bool TestInterface::initImpl()
{
    return true;
}

bool TestInterface::closeImpl()
{
    return true;
}
