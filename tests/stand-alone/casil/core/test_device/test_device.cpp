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

#include <casil/device.h>
#include <casil/layerbase.h>
#include <casil/TL/directinterface.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using casil::Device;

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(Device_Tests)

BOOST_AUTO_TEST_CASE(Test1_initClose)
{
    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface},"
                                        "{name: intf2, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1},"
                                    "{name: drv2, type: test_driver, interface: intf2}],"   //Using TestDriver from TemplateDevice test
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1},"
                                   "{name: reg2, type: DummyRegister, hw_driver: drv2}]}");

    BOOST_CHECK(exampleDev.init());
    BOOST_CHECK(exampleDev.init(false));
    BOOST_CHECK(exampleDev.init(true));

    BOOST_CHECK(exampleDev.interface("intf1").init(true));
    BOOST_CHECK(exampleDev.interface("intf2").init());
    BOOST_CHECK(exampleDev.driver("drv1").init(true));
    BOOST_CHECK(exampleDev.driver("drv2").init());
    BOOST_CHECK(exampleDev.reg("reg1").init(true));
    BOOST_CHECK(exampleDev.reg("reg2").init());

    BOOST_CHECK(dynamic_cast<casil::TL::DirectInterface&>(exampleDev.interface("intf1")).read() == std::vector<std::uint8_t>{});

    BOOST_CHECK(exampleDev.close());
    BOOST_CHECK(exampleDev.close(false));
    BOOST_CHECK(exampleDev.close(true));

    BOOST_CHECK(exampleDev.interface("intf1").close(true));
    BOOST_CHECK(exampleDev.interface("intf2").close());
    BOOST_CHECK(exampleDev.driver("drv1").close(true));
    BOOST_CHECK(exampleDev.driver("drv2").close());
    BOOST_CHECK(exampleDev.reg("reg1").close(true));
    BOOST_CHECK(exampleDev.reg("reg2").close());
}

BOOST_AUTO_TEST_CASE(Test2_layerTypeNameMatching)
{
    using casil::LayerBase;

    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface},"
                                        "{name: intf2, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1},"
                                    "{name: drv2, type: test_driver, interface: intf2}],"
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1},"
                                   "{name: reg2, type: DummyRegister, hw_driver: drv2}]}");

    BOOST_REQUIRE(exampleDev.init());

    BOOST_CHECK(exampleDev["intf1"].getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(exampleDev["intf1"].getType(), "DummyInterface");
    BOOST_CHECK_EQUAL(exampleDev["intf1"].getName(), "intf1");

    BOOST_CHECK(exampleDev["intf2"].getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(exampleDev["intf2"].getType(), "DummyInterface");
    BOOST_CHECK_EQUAL(exampleDev["intf2"].getName(), "intf2");

    BOOST_CHECK(exampleDev["drv1"].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev["drv1"].getType(), "DummyDriver");
    BOOST_CHECK_EQUAL(exampleDev["drv1"].getName(), "drv1");

    BOOST_CHECK(exampleDev["drv2"].getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(exampleDev["drv2"].getType(), "test_driver");
    BOOST_CHECK_EQUAL(exampleDev["drv2"].getName(), "drv2");

    BOOST_CHECK(exampleDev["reg1"].getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(exampleDev["reg1"].getType(), "DummyRegister");
    BOOST_CHECK_EQUAL(exampleDev["reg1"].getName(), "reg1");

    BOOST_CHECK(exampleDev["reg2"].getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(exampleDev["reg2"].getType(), "DummyRegister");
    BOOST_CHECK_EQUAL(exampleDev["reg2"].getName(), "reg2");

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test3_subscriptOperator)
{
    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface},"
                                        "{name: intf2, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1},"
                                    "{name: drv2, type: test_driver, interface: intf2}],"
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1},"
                                   "{name: reg2, type: DummyRegister, hw_driver: drv2}]}");

    BOOST_REQUIRE(exampleDev.init());

    BOOST_CHECK(&(exampleDev.interface("intf1")) == &(exampleDev["intf1"]));
    BOOST_CHECK(&(exampleDev.interface("intf2")) == &(exampleDev["intf2"]));
    BOOST_CHECK(&(exampleDev.driver("drv1")) == &(exampleDev["drv1"]));
    BOOST_CHECK(&(exampleDev.driver("drv2")) == &(exampleDev["drv2"]));
    BOOST_CHECK(&(exampleDev.reg("reg1")) == &(exampleDev["reg1"]));
    BOOST_CHECK(&(exampleDev.reg("reg2")) == &(exampleDev["reg2"]));

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test4_failedAccess)
{
    Device exampleDev("{transfer_layer: [], hw_drivers: [], registers: []}");

    BOOST_REQUIRE(exampleDev.init());

    int exceptionCtr = 0;

    try { exampleDev.interface("intf1").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev.interface("intf2").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev.driver("drv1").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev.driver("drv2").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev.reg("reg1").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev.reg("reg2").close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["intf1"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["intf2"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["drv1"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["drv2"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["reg1"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { exampleDev["reg2"].close(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 12);

    BOOST_CHECK(exampleDev.close());
}

BOOST_AUTO_TEST_CASE(Test5_interLayerDependency)
{
    int exceptionCtr = 0;

    try
    {
        //OK
        Device exampleDev("{transfer_layer: [{name: intf, type: DummyInterface}],"
                           "hw_drivers: [{name: drv, type: DummyDriver, interface: intf}],"
                           "registers: [{name: reg, type: DummyRegister, hw_driver: drv}]}");
        (void)exampleDev;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Register references to non-existent driver
        Device exampleDev("{transfer_layer: [{name: intf, type: DummyInterface}],"
                           "hw_drivers: [{name: drv, type: DummyDriver, interface: intf}],"
                           "registers: [{name: reg, type: DummyRegister, hw_driver: driver1}]}");
        (void)exampleDev;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Driver references to non-existent interface
        Device exampleDev("{transfer_layer: [{name: intf, type: DummyInterface}],"
                           "hw_drivers: [{name: drv, type: DummyDriver, interface: interface1}],"
                           "registers: [{name: reg, type: DummyRegister, hw_driver: drv}]}");
        (void)exampleDev;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Driver references to interface of wrong type
        Device exampleDev("{transfer_layer: [{name: intf, type: DummyInterface}],"
                           "hw_drivers: [{name: drv, type: GPIO, interface: intf}],"
                           "registers: [{name: reg, type: DummyRegister, hw_driver: drv}]}");
        (void)exampleDev;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 3);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
