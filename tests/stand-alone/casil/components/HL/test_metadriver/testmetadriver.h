/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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

#ifndef CASILTESTS_TESTMETADRIVER_H
#define CASILTESTS_TESTMETADRIVER_H

#include "testbackenddriver.h"

#include <casil/HL/metadriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers::HL
{

class TestMetaDriver final : public MetaDriver
{
public:
    TestMetaDriver(std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig);
    ~TestMetaDriver() override = default;
    //
    std::vector<std::uint8_t> computeReverse(const std::vector<std::uint8_t>& pData) const;
    std::uint64_t getCalibration() const;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    TestBackendDriver& testBackendDrv;

    CASIL_REGISTER_META_DRIVER_H("TestMetaDriver")
};

} // namespace HL

} // namespace casil

#endif // CASILTESTS_TESTMETADRIVER_H
