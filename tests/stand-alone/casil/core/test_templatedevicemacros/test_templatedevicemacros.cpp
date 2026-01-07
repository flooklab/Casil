/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2026 M. Frohne
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

#include <casil/device.h>
#include <casil/layerbase.h>
#include <casil/templatedevice.h>
#include <casil/templatedevicemacros.h>
#include <casil/HL/Direct/dummydriver.h>
#include <casil/HL/Muxed/dummymetadriver.h>
#include <casil/HL/Muxed/dummymuxeddriver.h>
#include <casil/RL/dummyregister.h>
#include <casil/TL/Direct/dummyinterface.h>
#include <casil/TL/Muxed/dummymuxedinterface.h>

#include <cstdint>
#include <vector>

CASIL_DEFINE_INTERFACE(casil::TL::DummyInterface,
                       TLDummyInterface1,
                       "DummyInterface1",
                       "")

CASIL_DEFINE_DRIVER(casil::HL::DummyDriver,
                    HLDummyDriver1,
                    "DummyDriver1",
                    "DummyInterface1",
                    "")

CASIL_DEFINE_REGISTER(casil::RL::DummyRegister,
                      RLDummyRegister1,
                      "DummyRegister1",
                      "DummyDriver1",
                      "")

typedef casil::TemplateDevice<
                casil::TmplDev::InterfacesConf<TLDummyInterface1>,
                casil::TmplDev::DriversConf<HLDummyDriver1>,
                casil::TmplDev::RegistersConf<RLDummyRegister1>
            > ExampleDevice;

CASIL_DEFINE_INTERFACE(casil::TL::DummyMuxedInterface,
                       TLDummyInterface2,
                       "DummyInterface2",
                       "")

CASIL_DEFINE_DRIVER(casil::HL::DummyMuxedDriver,
                    HLDummyDriver2,
                    "DummyDriver2",
                    "DummyInterface2",
                    "base_addr: 0x123")

CASIL_DEFINE_META_DRIVER(casil::HL::DummyMetaDriver,
                    HLMetaDriver1,
                    "MetaDriver1",
                    "DummyDriver2",
                    "")

CASIL_DEFINE_REGISTER(casil::RL::DummyRegister,
                      RLDummyRegister2,
                      "DummyRegister2",
                      "MetaDriver1",
                      "")

typedef casil::TemplateDevice<
                casil::TmplDev::InterfacesConf<TLDummyInterface2>,
                casil::TmplDev::DriversConf<HLDummyDriver2,
                                            HLMetaDriver1>,
                casil::TmplDev::RegistersConf<RLDummyRegister2>
            > ExampleDevice2;

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(TemplateDeviceMacros_Tests)

BOOST_AUTO_TEST_CASE(Test1_initCloseAndTemplateAccess)
{
    ExampleDevice exampleDev;

    BOOST_CHECK(exampleDev.init());
    BOOST_CHECK(exampleDev.init(false));
    BOOST_CHECK(exampleDev.init(true));

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().init());
    BOOST_CHECK(exampleDev.driver<HLDummyDriver1>().init());
    BOOST_CHECK(exampleDev.reg<RLDummyRegister1>().init());

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().read() == std::vector<std::uint8_t>{});

    BOOST_CHECK(exampleDev.close());
    BOOST_CHECK(exampleDev.close(false));
    BOOST_CHECK(exampleDev.close(true));

    BOOST_CHECK(exampleDev.interface<TLDummyInterface1>().close());
    BOOST_CHECK(exampleDev.driver<HLDummyDriver1>().close());
    BOOST_CHECK(exampleDev.reg<RLDummyRegister1>().close());

    ExampleDevice2 exampleDev2;

    BOOST_CHECK(exampleDev2.init());
    BOOST_CHECK(exampleDev2.init(false));
    BOOST_CHECK(exampleDev2.init(true));

    BOOST_CHECK(exampleDev2.interface<TLDummyInterface2>().init());
    BOOST_CHECK(exampleDev2.driver<HLDummyDriver2>().init());
    BOOST_CHECK(exampleDev2.driver<HLMetaDriver1>().init());
    BOOST_CHECK(exampleDev2.reg<RLDummyRegister2>().init());

    BOOST_CHECK(exampleDev2.interface<TLDummyInterface2>().read(0x0u) == std::vector<std::uint8_t>{});

    BOOST_CHECK(exampleDev2.close());
    BOOST_CHECK(exampleDev2.close(false));
    BOOST_CHECK(exampleDev2.close(true));

    BOOST_CHECK(exampleDev2.interface<TLDummyInterface2>().close());
    BOOST_CHECK(exampleDev2.driver<HLDummyDriver2>().close());
    BOOST_CHECK(exampleDev2.driver<HLMetaDriver1>().close());
    BOOST_CHECK(exampleDev2.reg<RLDummyRegister2>().close());
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

    BOOST_CHECK(exampleDev[RLDummyRegister1::name].getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(exampleDev[RLDummyRegister1::name].getType(), RLDummyRegister1::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev[RLDummyRegister1::name].getName(), RLDummyRegister1::name);

    exampleDev.close();

    ExampleDevice2 exampleDev2;

    BOOST_REQUIRE(exampleDev2.init());

    BOOST_CHECK(exampleDev2[TLDummyInterface2::name].getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(exampleDev2[TLDummyInterface2::name].getType(), TLDummyInterface2::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev2[TLDummyInterface2::name].getName(), TLDummyInterface2::name);

    BOOST_CHECK(exampleDev2[HLDummyDriver2::name].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev2[HLDummyDriver2::name].getType(), HLDummyDriver2::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev2[HLDummyDriver2::name].getName(), HLDummyDriver2::name);

    BOOST_CHECK(exampleDev2[HLMetaDriver1::name].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev2[HLMetaDriver1::name].getType(), HLMetaDriver1::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev2[HLMetaDriver1::name].getName(), HLMetaDriver1::name);

    BOOST_CHECK(exampleDev2[RLDummyRegister2::name].getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(exampleDev2[RLDummyRegister2::name].getType(), RLDummyRegister2::Type::typeName);
    BOOST_CHECK_EQUAL(exampleDev2[RLDummyRegister2::name].getName(), RLDummyRegister2::name);

    exampleDev2.close();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
