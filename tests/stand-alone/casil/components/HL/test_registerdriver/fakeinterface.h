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

#ifndef CASILTESTS_FAKEINTERFACE_H
#define CASILTESTS_FAKEINTERFACE_H

#include <casil/TL/muxedinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::TL
{

class FakeInterface final : public MuxedInterface
{
public:
    FakeInterface(std::string pName, LayerConfig pConfig);
    ~FakeInterface() override = default;
    //
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) override;
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;
    //
    void fakeFirmwareVersion();

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    std::array<std::uint8_t, 46> buffer;
    std::uint8_t firmwareVersion;

private:
    static constexpr std::uint64_t drvBaseAddr = 0x135Fu;

    CASIL_REGISTER_INTERFACE_H("FakeInterface")
};

} // namespace TL

} // namespace casil

#endif // CASILTESTS_FAKEINTERFACE_H
