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

#include "testmetadriver.h"

#include <utility>

using casil::HL::TestMetaDriver;

CASIL_REGISTER_META_DRIVER_CPP(TestMetaDriver)

//

TestMetaDriver::TestMetaDriver(std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig) :
    MetaDriver(typeName, std::move(pName), pBackendDriver, std::move(pConfig), LayerConfig()),
    testBackendDrv(dynamic_cast<TestBackendDriver&>(backendDriver)) //Possible exception will be caught by macro-registered factory generator
{
}

//Public

std::vector<std::uint8_t> TestMetaDriver::computeReverse(const std::vector<std::uint8_t>& pData) const
{
    backendDriver.setData(pData);
    backendDriver.exec();
    if (backendDriver.isDone())
        return backendDriver.getData();
    else
        return {};
}

std::uint64_t TestMetaDriver::getCalibration() const
{
    return testBackendDrv.getSomeSpecialValue() - 0xCC2u;
}

//Private

bool TestMetaDriver::initImpl()
{
    return true;
}

bool TestMetaDriver::closeImpl()
{
    return true;
}
