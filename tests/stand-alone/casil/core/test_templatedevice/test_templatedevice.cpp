/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#include "exampledevice.h"

#include <casil/device.h>
#include <casil/layerbase.h>
#include <casil/templatedevice.h>

#include <cstdint>
#include <vector>

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(TemplateDevice_Tests)

BOOST_AUTO_TEST_CASE(Test1_initCloseAndTemplateAccess)
{
    ExampleDevice exampleDev;

    BOOST_CHECK(exampleDev.init());
    BOOST_CHECK(exampleDev.init(false));
    BOOST_CHECK(exampleDev.init(true));

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().init());
    BOOST_CHECK(exampleDev.driver<HLDummyDriver1>().init());
    BOOST_CHECK(exampleDev.driver<HLDriver2>().init());
    BOOST_CHECK(exampleDev.reg<RLDummyRegister1>().init());

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().read() == std::vector<std::uint8_t>{});

    BOOST_CHECK(exampleDev.close());
    BOOST_CHECK(exampleDev.close(false));
    BOOST_CHECK(exampleDev.close(true));

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().close());
    BOOST_CHECK(exampleDev.driver<HLDummyDriver1>().close());
    BOOST_CHECK(exampleDev.driver<HLDriver2>().close());
    BOOST_CHECK(exampleDev.reg<RLDummyRegister1>().close());
}

BOOST_AUTO_TEST_CASE(Test2_layerTypeNameMatching)
{
    using casil::LayerBase;

    ExampleDevice exampleDev;

    BOOST_REQUIRE(exampleDev.init());

    BOOST_CHECK(exampleDev[TLDummyInterface1::name].getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(exampleDev[TLDummyInterface1::name].getType(), TLDummyInterface1::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev[TLDummyInterface1::name].getName(), TLDummyInterface1::name);

    BOOST_CHECK(exampleDev[HLDummyDriver1::name].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev[HLDummyDriver1::name].getType(), HLDummyDriver1::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev[HLDummyDriver1::name].getName(), HLDummyDriver1::name);

    BOOST_CHECK(exampleDev[HLDriver2::name].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev[HLDriver2::name].getType(), HLDriver2::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev[HLDriver2::name].getName(), HLDriver2::name);

    BOOST_CHECK(exampleDev[RLDummyRegister1::name].getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(exampleDev[RLDummyRegister1::name].getType(), RLDummyRegister1::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev[RLDummyRegister1::name].getName(), RLDummyRegister1::name);

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test3_subscriptOperator)
{
    ExampleDevice exampleDev;

    BOOST_REQUIRE(exampleDev.init());

    BOOST_CHECK(&(exampleDev[TLDummyInterface1::name]) == &(exampleDev.interface<TLDummyInterface1>()));
    BOOST_CHECK(&(exampleDev[HLDummyDriver1::name]) == &(exampleDev.driver<HLDummyDriver1>()));
    BOOST_CHECK(&(exampleDev[HLDriver2::name]) == &(exampleDev.driver<HLDriver2>()));
    BOOST_CHECK(&(exampleDev[RLDummyRegister1::name]) == &(exampleDev.reg<RLDummyRegister1>()));

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test4_dynamicCastToPlainDevice)
{
    ExampleDevice exampleDev;

    BOOST_REQUIRE(exampleDev.init());

    BOOST_CHECK(dynamic_cast<casil::Device*>(&exampleDev)->driver("testdrv123").init(true));

    exampleDev.close();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
