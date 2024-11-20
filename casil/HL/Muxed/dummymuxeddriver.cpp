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

#include <casil/HL/Muxed/dummymuxeddriver.h>

#include <casil/bytes.h>
#include <casil/logger.h>

#include <utility>

using casil::HL::DummyMuxedDriver;

CASIL_REGISTER_DRIVER_CPP(DummyMuxedDriver)

//

DummyMuxedDriver::DummyMuxedDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    MuxedDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Public

std::vector<std::uint8_t> DummyMuxedDriver::getData(const int pSize, const std::uint32_t pAddrOffs)
{
    Logger::logDebug("Called getData() for " + getSelfDescription() + " with arguments " +
                     "\"pSize\" = " + std::to_string(pSize) + ", " +
                     "\"pAddrOffs\" = " + Bytes::formatHex(pAddrOffs) + ".");
    return {};
}

void DummyMuxedDriver::setData(const std::vector<std::uint8_t>& pData, const std::uint32_t pAddrOffs)
{
    Logger::logDebug("Called setData() for " + getSelfDescription() + " with arguments " +
                     "\"pData\" = " + Bytes::formatByteVec(pData) + ", " +
                     "\"pAddrOffs\" = " + Bytes::formatHex(pAddrOffs) + ".");
}

void DummyMuxedDriver::exec()
{
    Logger::logDebug("Called exec() for " + getSelfDescription() + ".");
}

bool DummyMuxedDriver::isDone()
{
    Logger::logDebug("Called isDone() for " + getSelfDescription() + ".");
    return false;
}

//Private

bool DummyMuxedDriver::initImpl()
{
    Logger::logDebug("Called initImpl() for " + getSelfDescription() + ".");
    return true;
}

bool DummyMuxedDriver::closeImpl()
{
    Logger::logDebug("Called closeImpl() for " + getSelfDescription() + ".");
    return true;
}
