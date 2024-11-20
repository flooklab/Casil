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

#include <casil/TL/muxedinterface.h>

#include <casil/logger.h>

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>

using casil::TL::MuxedInterface;

MuxedInterface::MuxedInterface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    Interface(std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig)
{
}

//Public

std::vector<std::uint8_t> MuxedInterface::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                                const std::vector<std::uint8_t>& pData, const int pSize)
{
    try
    {
        if (!readBufferEmpty())
        {
            Logger::logWarning("Clearing not empty read buffer of " + getSelfDescription() + " before sending query.");
            clearReadBuffer();
        }

        write(pWriteAddr, pData);

        if (queryDelayMicroSecs > std::chrono::microseconds::zero())
            std::this_thread::sleep_for(queryDelayMicroSecs);

        return read(pReadAddr, pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not query from " + getSelfDescription() + ": " + exc.what());
    }
}
