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

#include <casil/HL/muxeddriver.h>

#include <casil/bytes.h>

#include <stdexcept>
#include <utility>

using casil::HL::MuxedDriver;

MuxedDriver::MuxedDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                         LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    Driver(std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig),
    interface(pInterface),
    baseAddr(config.getUInt("base_addr", 0x0u))
{
    if (!config.contains(LayerConfig::fromYAML("{base_addr: uint}"), true))
        throw std::runtime_error("Invalid or no base address (\"base_addr\") set for " + getSelfDescription() + ".");
}

//Public

std::vector<std::uint8_t> MuxedDriver::getData(int, std::uint32_t)
{
    return {};  //(sic!)
}

void MuxedDriver::setData(const std::vector<std::uint8_t>&, std::uint32_t)
{
    //(sic!)
}

void MuxedDriver::exec()
{
    //(sic!)
}

bool MuxedDriver::isDone()
{
    return false;   //(sic!)
}

//Protected

std::vector<std::uint8_t> MuxedDriver::read(const std::uint64_t pAddr, const int pSize) const
{
    try
    {
        return interface.read(baseAddr + pAddr, pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to read from interface (address: " + Bytes::formatHex(pAddr) +
                                 ", size: " + std::to_string(pSize) + "): " + exc.what());
    }
}

void MuxedDriver::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) const
{
    try
    {
        interface.write(baseAddr + pAddr, pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to write to interface (address: " + Bytes::formatHex(pAddr) +
                                 ", data: " + Bytes::formatByteVec(pData) + "): " + exc.what());
    }
}

std::vector<std::uint8_t> MuxedDriver::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                             const std::vector<std::uint8_t>& pData, const int pSize) const
{
    try
    {
        return interface.query(baseAddr + pWriteAddr, baseAddr + pReadAddr, pData, pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to query from interface (write address: " +
                                 Bytes::formatHex(pWriteAddr) + ", read address: " + Bytes::formatHex(pReadAddr) +
                                 ", data: " + Bytes::formatByteVec(pData) + ", size: " + std::to_string(pSize) + "): " + exc.what());
    }
}