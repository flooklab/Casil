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

#ifndef CASIL_LAYERS_TL_DUMMYINTERFACE_H
#define CASIL_LAYERS_TL_DUMMYINTERFACE_H

#include <casil/TL/directinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::TL
{

/*!
 * \brief Dummy implementation of DirectInterface without actual functionality.
 */
class DummyInterface final : public DirectInterface
{
public:
    DummyInterface(std::string pName, LayerConfig pConfig);     ///< Constructor.
    ~DummyInterface() override = default;                       ///< Default destructor.
    //
    std::vector<std::uint8_t> read(int pSize = -1) override;
    void write(const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_INTERFACE_H("DummyInterface")
};

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_DUMMYINTERFACE_H
