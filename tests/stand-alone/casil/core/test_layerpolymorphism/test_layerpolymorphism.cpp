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

#include "wrongregister.h"
#include "testdrivermuxed.h"

#include <casil/device.h>
#include <casil/layerbase.h>
#include <casil/HL/directdriver.h>
#include <casil/HL/driver.h>
#include <casil/HL/muxeddriver.h>
#include <casil/HL/Direct/dummydriver.h>
#include <casil/HL/Direct/virtecho.h>
#include <casil/RL/register.h>
#include <casil/RL/dummyregister.h>
#include <casil/TL/directinterface.h>
#include <casil/TL/interface.h>
#include <casil/TL/muxedinterface.h>
#include <casil/TL/Direct/dummyinterface.h>
#include <casil/TL/Direct/tcp.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using casil::Device;
using casil::LayerBase;
using casil::TL::Interface;
using casil::TL::DirectInterface;
using casil::TL::MuxedInterface;
using casil::TL::DummyInterface;
using casil::HL::Driver;
using casil::HL::DirectDriver;
using casil::HL::MuxedDriver;
using casil::HL::DummyDriver;
using casil::RL::Register;
using casil::RL::DummyRegister;

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(LayerPolymorphism_Tests)

BOOST_AUTO_TEST_CASE(Test1_dynamicDowncastsValid)
{
    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1}],"
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1}]}");

    BOOST_REQUIRE(exampleDev.init());

    LayerBase& intfL0 = exampleDev["intf1"];
    LayerBase& drvL0 = exampleDev["drv1"];
    LayerBase& regL0 = exampleDev["reg1"];

    BOOST_CHECK(intfL0.close() == true);
    BOOST_CHECK(drvL0.close() == true);
    BOOST_CHECK(regL0.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    Interface& intfL1 = dynamic_cast<Interface&>(intfL0);
    Driver& drvL1 = dynamic_cast<Driver&>(drvL0);
    Register& regL1 = dynamic_cast<Register&>(regL0);

    BOOST_CHECK(intfL1.close() == true);
    BOOST_CHECK(drvL1.close() == true);
    BOOST_CHECK(regL1.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    DirectInterface& intfL2 = dynamic_cast<DirectInterface&>(intfL1);
    DirectDriver& drvL2 = dynamic_cast<DirectDriver&>(drvL1);

    BOOST_CHECK(intfL2.close() == true);
    BOOST_CHECK(drvL2.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    DummyInterface& intfL3 = dynamic_cast<DummyInterface&>(intfL2);
    DummyDriver& drvL3 = dynamic_cast<DummyDriver&>(drvL2);
    DummyRegister& regL3 = dynamic_cast<DummyRegister&>(regL1);

    BOOST_CHECK(intfL3.close() == true);
    BOOST_CHECK(drvL3.close() == true);
    BOOST_CHECK(regL3.close() == true);
}

BOOST_AUTO_TEST_CASE(Test2_dynamicDowncastsInvalid)
{
    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1}],"
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1}]}");

    BOOST_REQUIRE(exampleDev.init());

    LayerBase& intfL0 = exampleDev["intf1"];
    LayerBase& drvL0 = exampleDev["drv1"];
    LayerBase& regL0 = exampleDev["reg1"];

    BOOST_REQUIRE(intfL0.close() == true);
    BOOST_REQUIRE(drvL0.close() == true);
    BOOST_REQUIRE(regL0.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    //Check some valid casts first

    DummyInterface& intfL3 = dynamic_cast<DummyInterface&>(intfL0);
    DummyDriver& drvL3 = dynamic_cast<DummyDriver&>(drvL0);
    DummyRegister& regL3 = dynamic_cast<DummyRegister&>(regL0);

    BOOST_CHECK(intfL3.close() == true);
    BOOST_CHECK(drvL3.close() == true);
    BOOST_CHECK(regL3.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    //Invalid casts to wrong components of correct layer

    BOOST_CHECK(dynamic_cast<casil::TL::TCP*>(&intfL0) == nullptr);
    BOOST_CHECK(dynamic_cast<casil::HL::VirtEcho*>(&drvL0) == nullptr);
    BOOST_CHECK(dynamic_cast<casil::WrongRegister*>(&regL0) == nullptr);

    //Invalid casts to components of wrong layer

    BOOST_CHECK(dynamic_cast<DummyDriver*>(&intfL0) == nullptr);
    BOOST_CHECK(dynamic_cast<DummyRegister*>(&intfL0) == nullptr);
    BOOST_CHECK(dynamic_cast<DummyInterface*>(&drvL0) == nullptr);
    BOOST_CHECK(dynamic_cast<DummyRegister*>(&drvL0) == nullptr);
    BOOST_CHECK(dynamic_cast<DummyInterface*>(&regL0) == nullptr);
    BOOST_CHECK(dynamic_cast<DummyDriver*>(&regL0) == nullptr);

    //Invalid casts to wrong base types of correct layer

    BOOST_CHECK(dynamic_cast<MuxedInterface*>(&intfL0) == nullptr);
    BOOST_CHECK(dynamic_cast<MuxedDriver*>(&drvL0) == nullptr);

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test3_dynamicUpcastsInvalid)
{
    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1}],"
                       "registers: [{name: reg1, type: DummyRegister, hw_driver: drv1}]}");

    BOOST_REQUIRE(exampleDev.init());

    DummyInterface& intf = dynamic_cast<DummyInterface&>(exampleDev.interface("intf1"));
    DummyDriver& drv = dynamic_cast<DummyDriver&>(exampleDev.driver("drv1"));
    DummyRegister& reg = dynamic_cast<DummyRegister&>(exampleDev.reg("reg1"));

    BOOST_REQUIRE(intf.close() == true);
    BOOST_REQUIRE(drv.close() == true);
    BOOST_REQUIRE(reg.close() == true);

    BOOST_REQUIRE(exampleDev.close());
    BOOST_REQUIRE(exampleDev.init());

    //Check some valid casts first

    BOOST_CHECK(dynamic_cast<DirectInterface*>(&intf) != nullptr);
    BOOST_CHECK(dynamic_cast<DirectDriver*>(&drv) != nullptr);

    BOOST_CHECK(dynamic_cast<Interface*>(&intf) != nullptr);
    BOOST_CHECK(dynamic_cast<Driver*>(&drv) != nullptr);
    BOOST_CHECK(dynamic_cast<Register*>(&reg) != nullptr);

    //Invalid casts to wrong base types of correct layer

    BOOST_CHECK(dynamic_cast<MuxedInterface*>(&intf) == nullptr);
    BOOST_CHECK(dynamic_cast<MuxedDriver*>(&drv) == nullptr);

    //Invalid casts to base types of wrong layer

    BOOST_CHECK(dynamic_cast<Driver*>(&intf) == nullptr);
    BOOST_CHECK(dynamic_cast<Interface*>(&drv) == nullptr);

    exampleDev.close();
}

BOOST_AUTO_TEST_CASE(Test4_polymorphicMemberAccess)
{
    using casil::TestDriverMuxed;

    Device exampleDev("{transfer_layer: [{name: intf1, type: DummyInterface},"
                                        "{name: intf2, type: test_interface}],"
                       "hw_drivers: [{name: drv1, type: DummyDriver, interface: intf1},"
                                    "{name: drv2, type: test_driver_muxed, interface: intf2, base_addr: 0x0}],"
                       "registers: []}");

    BOOST_REQUIRE(exampleDev.init());

    Driver& drv1 = exampleDev.driver("drv1");
    Driver& drv2 = exampleDev.driver("drv2");

    BOOST_CHECK(drv1.getData() == std::vector<std::uint8_t>{});
    BOOST_CHECK(drv2.getData() == (std::vector<std::uint8_t>{1, 1, 2, 3, 5, 8, 13}));

    drv1.setData({});
    drv2.setData({0, 0, 0});

    std::string excStr;

    try
    {
        drv2.setData({1, 2, 3, 4, 5});
    }
    catch (const std::runtime_error& exc)
    {
        excStr = exc.what();
    }

    BOOST_CHECK_EQUAL(excStr, "This exception is a test...");

    BOOST_CHECK(dynamic_cast<TestDriverMuxed&>(drv2).someSpecialFunctionality(0) == false);
    BOOST_CHECK(dynamic_cast<TestDriverMuxed&>(drv2).someSpecialFunctionality(123) == true);

    exampleDev.close();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
