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

#ifndef CASIL_LAYERS_TL_DUMMYMUXEDINTERFACE_H
#define CASIL_LAYERS_TL_DUMMYMUXEDINTERFACE_H

#include <casil/TL/muxedinterface.h>

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
 * \brief Dummy implementation of MuxedInterface without actual functionality.
 */
class DummyMuxedInterface final : public MuxedInterface
{
public:
    DummyMuxedInterface(std::string pName, LayerConfig pConfig);    ///< Constructor.
    ~DummyMuxedInterface() override = default;                      ///< Default destructor.
    //
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) override;
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_INTERFACE_H("DummyMuxedInterface")
};

} // namespace Layers::TL

} // namespace casil

#endif // CASIL_LAYERS_TL_DUMMYMUXEDINTERFACE_H
