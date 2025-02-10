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

#include "fakeinterface.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

using casil::TL::FakeInterface;

CASIL_REGISTER_INTERFACE_CPP(FakeInterface)

//

FakeInterface::FakeInterface(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig()),
    buffer({0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u,
            0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u,
            0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u}),
    firmwareVersion(11)
{
}

//Public

std::vector<std::uint8_t> FakeInterface::read(const std::uint64_t pAddr, const int pSize)
{
    if (pAddr < drvBaseAddr)
        throw std::invalid_argument("Negative module address.");

    if (pSize <= 0)
        throw std::invalid_argument("Read size should be positive.");

    const std::uint64_t regAddr = pAddr - drvBaseAddr;

    if (regAddr + pSize > 46)
        throw std::invalid_argument("Read range exceeds buffer size.");

    if (regAddr == 0 && pSize == 1)
        return {firmwareVersion};
    else if (regAddr == 3 && pSize == 1)
        return {0b10110001};    //Return 'INPUT'
    else if ((regAddr == 1 && pSize == 2) || (regAddr == 4 && pSize == 3) || (regAddr == 9 && pSize == 2) || (regAddr == 11 && pSize == 2) ||
             (regAddr == 13 && pSize == 3) || (regAddr == 16 && pSize == 2) || (regAddr == 18 && pSize == 2) ||
             (regAddr == 20 && pSize == 4) || (regAddr == 24 && pSize == 5) || (regAddr == 29 && pSize == 8) || (regAddr == 37 && pSize == 9))
    {
        return std::vector<std::uint8_t>(buffer.begin()+regAddr, buffer.begin()+regAddr+pSize);
    }
    else
        throw std::invalid_argument("Invalid combination of address and read size.");
}

void FakeInterface::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    if (pAddr  < drvBaseAddr)
        throw std::invalid_argument("Negative module address.");

    if (pData.size() == 0)
        throw std::invalid_argument("Write data are empty.");

    const std::uint64_t regAddr = pAddr - drvBaseAddr;

    if (regAddr + pData.size() > 46)
        throw std::invalid_argument("Write range exceeds buffer size.");

    if (regAddr == 0 && pData.size() == 1)
    {
        //Reset
        buffer = {0x0u, 0x12u, 0x34u, 0x0u, 0x56u, 0x78u, 0x9Au, 0x0u, 0x0u, 0x14u, 0x23u, 0x58u, 0x67u, 0x0u,
                  0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u,
                  0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u, 0x0u};
    }
    else if ((regAddr == 1 && pData.size() == 2) || (regAddr == 4 && pData.size() == 3) ||
             (regAddr == 9 && pData.size() == 2) || (regAddr == 11 && pData.size() == 2) ||
             (regAddr == 13 && pData.size() == 3) || (regAddr == 16 && pData.size() == 2) || (regAddr == 18 && pData.size() == 2) ||
             (regAddr == 20 && pData.size() == 4) || (regAddr == 24 && pData.size() == 5) ||
             (regAddr == 29 && pData.size() == 8) || (regAddr == 37 && pData.size() == 9))
    {
        for (std::size_t i = 0; i < pData.size(); ++i)
            buffer[regAddr+i] = pData[i];
    }
    else if (regAddr == 7 && pData.size() == 2)
        ;   //Do nothing
    else
        throw std::invalid_argument("Invalid combination of address and write data size.");
}

std::vector<std::uint8_t> FakeInterface::query(std::uint64_t, std::uint64_t, const std::vector<std::uint8_t>&, int)
{
    return {};
}

//

bool FakeInterface::readBufferEmpty() const
{
    return true;
}

void FakeInterface::clearReadBuffer()
{
}

//

void FakeInterface::fakeFirmwareVersion()
{
    firmwareVersion = 74;
}

//Private

bool FakeInterface::initImpl()
{
    return true;
}

bool FakeInterface::closeImpl()
{
    return true;
}
