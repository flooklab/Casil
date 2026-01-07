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

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/HL/metadriver.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using casil::Device;
using casil::HL::TestMetaDriver;

namespace Bytes = casil::Bytes;

namespace boost { using Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(MetaDriver_Tests)

BOOST_AUTO_TEST_CASE(Test1_wrongBackendDriver)
{
    int exceptionCtr = 0;

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_REQUIRE_EQUAL(exceptionCtr, 0);

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: bDrv2, type: GPIO, interface: intf, base_addr: 0x100, size: 8}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv2}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        Device d("{transfer_layer: [{name: intf2, type: DummyInterface}],"
                  "hw_drivers: [{name: bDrv2, type: SCPI, interface: intf2, init: {device: \"Keithley 2400\"}}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv2}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 2);
}

BOOST_AUTO_TEST_CASE(Test2_initClose)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                           "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}], "
              "registers: []}");

    BOOST_CHECK(d.init());
    BOOST_CHECK(d.close());
}

BOOST_AUTO_TEST_CASE(Test3_useBackendDriver)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                           "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}], "
              "registers: []}");

    BOOST_REQUIRE(d.init());

    TestMetaDriver& metaDrv = dynamic_cast<TestMetaDriver&>(d.driver("mDrv"));

    BOOST_CHECK_EQUAL((metaDrv.computeReverse({0xFFu, 0x00u, 0xFFu, 0x54u, 0x7Au, 0xFFu, 0xDFu})),
                      (std::vector<std::uint8_t>{0xDFu, 0xFFu, 0x7Au, 0x54u, 0xFFu, 0x00u, 0xFFu}));

    BOOST_CHECK_EQUAL(metaDrv.getCalibration(), 1988275068121931400u);

    BOOST_CHECK(d.close());
}

BOOST_AUTO_TEST_CASE(Test4_wrongDriverOrder)
{
    int exceptionCtr = 0;

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_REQUIRE_EQUAL(exceptionCtr, 0);

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}, "
                               "{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 1);
}

BOOST_AUTO_TEST_CASE(Test5_baseAddrException)
{
    bool exceptionThrown = false;

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { exceptionThrown = true; }

    BOOST_REQUIRE(exceptionThrown == false);

    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: bDrv, type: TestBackendDriver, interface: intf, base_addr: 0x100}, "
                               "{name: mDrv, type: TestMetaDriver, hw_driver: bDrv, base_addr: 0x000}], "
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { exceptionThrown = true; }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
