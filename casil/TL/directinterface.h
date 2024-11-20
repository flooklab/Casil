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

#ifndef CASIL_TL_DIRECTINTERFACE_H
#define CASIL_TL_DIRECTINTERFACE_H

#include <casil/TL/interface.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace TL
{

class DirectInterface : public Interface
{
public:
    DirectInterface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
    ~DirectInterface() override = default;
    //
    virtual std::vector<std::uint8_t> read(int pSize = -1) = 0;
    virtual void write(const std::vector<std::uint8_t>& pData) = 0;
    virtual std::vector<std::uint8_t> query(const std::vector<std::uint8_t>& pData, int pSize = -1) = 0;
    //
    bool readBufferEmpty() const override = 0;
    void clearReadBuffer() override = 0;

private:
    bool initImpl() override = 0;
    bool closeImpl() override = 0;
};

} // namespace TL

} // namespace casil

#endif // CASIL_TL_DIRECTINTERFACE_H
