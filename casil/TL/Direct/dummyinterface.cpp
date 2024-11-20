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

#include <casil/TL/Direct/dummyinterface.h>

#include <casil/bytes.h>
#include <casil/logger.h>

#include <utility>

using casil::TL::DummyInterface;

CASIL_REGISTER_INTERFACE_CPP(DummyInterface)

//

DummyInterface::DummyInterface(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig())
{
}

//Public

std::vector<std::uint8_t> DummyInterface::read(const int pSize)
{
    Logger::logDebug("Called read() for " + getSelfDescription() + " with argument \"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

void DummyInterface::write(const std::vector<std::uint8_t>& pData)
{
    Logger::logDebug("Called write() for " + getSelfDescription() + " with argument \"pData\" = " + Bytes::formatByteVec(pData) + ".");
}

std::vector<std::uint8_t> DummyInterface::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    Logger::logDebug("Called query() for " + getSelfDescription() + " with arguments " +
                     "\"pData\" = " + Bytes::formatByteVec(pData) + ", " +
                     "\"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

//

bool DummyInterface::readBufferEmpty() const
{
    Logger::logDebug("Called readBufferEmpty() for " + getSelfDescription() + ".");
    return true;
}

void DummyInterface::clearReadBuffer()
{
    Logger::logDebug("Called clearReadBuffer() for " + getSelfDescription() + ".");
}

//Private

bool DummyInterface::initImpl()
{
    Logger::logDebug("Called initImpl() for " + getSelfDescription() + ".");
    return true;
}

bool DummyInterface::closeImpl()
{
    Logger::logDebug("Called closeImpl() for " + getSelfDescription() + ".");
    return true;
}
