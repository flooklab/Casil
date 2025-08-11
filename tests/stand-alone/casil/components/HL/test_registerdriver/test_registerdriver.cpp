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

#include "testregdriver.h"
#include "fakeinterface.h"

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/HL/driver.h>
#include <casil/HL/registerdriver.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using casil::Device;
using casil::HL::Driver;
using casil::HL::RegisterDriver;
using casil::HL::TestRegDriver;
using casil::TL::FakeInterface;

namespace Bytes = casil::Bytes;

namespace boost { using Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(RegisterDriver_Tests)

BOOST_AUTO_TEST_CASE(Test1_invalidRegisterDefinitions)
{
    static constexpr int numFailureModes = 8;

    int exceptionCtr = 0;

    for (int i = -1; i < numFailureModes; ++i)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                      "hw_drivers: [{name: drv, type: InvalidRegDriver, interface: intf, base_addr: 0x0, failureMode: " +
                      std::to_string(i) + "}], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            ++exceptionCtr;
        }
    }

    BOOST_CHECK_EQUAL(exceptionCtr, numFailureModes);
}

BOOST_AUTO_TEST_CASE(Test2_invalidRegisterDefaultOverrides)
{
    int exceptionCtr = 0;

    try
    {
        //No failure
        Device d("{transfer_layer: [{name: intf2, type: FakeInterface}],"
                  "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                                "init: { FOOBAR: 0x313, OUTPUT: [0x11, 0x22, 0x44] }}],"
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Default type mismatch
        Device d("{transfer_layer: [{name: intf2, type: FakeInterface}],"
                  "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                                "init: { OUTPUT: 0x313 }}],"
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Default type mismatch
        Device d("{transfer_layer: [{name: intf2, type: FakeInterface}],"
                  "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                                "init: { FOOBAR: [0x11, 0x22, 0x44] }}],"
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Default length mismatch
        Device d("{transfer_layer: [{name: intf2, type: FakeInterface}],"
                  "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                                "init: { OUTPUT: [0x11, 0x22] }}],"
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        //Invalid type
        Device d("{transfer_layer: [{name: intf2, type: FakeInterface}],"
                  "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                                "init: { OUTPUT: abc }}],"
                  "registers: []}");
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 4);
}

BOOST_AUTO_TEST_CASE(Test3_initModuleCloseModule)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.interface("intf").init());

    Driver& drv = d.driver("drv");

    //See counter logic in TestRegDriver
    BOOST_CHECK_EQUAL(drv.init(), true);
    BOOST_CHECK_EQUAL(drv.init(true), true);
    BOOST_CHECK_EQUAL(drv.init(true), false);
    BOOST_CHECK_EQUAL(drv.init(true), false);

    //See counter logic in TestRegDriver
    BOOST_CHECK_EQUAL(drv.close(true), true);
    BOOST_CHECK_EQUAL(drv.close(true), true);
    BOOST_CHECK_EQUAL(drv.close(true), false);
    BOOST_CHECK_EQUAL(drv.close(true), false);
}

BOOST_AUTO_TEST_CASE(Test4_firmwareVersion)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.interface("intf").init());

    FakeInterface& intf = dynamic_cast<FakeInterface&>(d.interface("intf"));
    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK_EQUAL(drv.init(), true);

    intf.fakeFirmwareVersion();

    BOOST_CHECK_EQUAL(drv.init(true), false);
}

BOOST_AUTO_TEST_CASE(Test5_readDefaults)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK_EQUAL(drv.getValue("VERSION"), 11);
    BOOST_CHECK_EQUAL(drv.getValue("FOOBAR"), 582);
    BOOST_CHECK_EQUAL(drv.getBytes("INPUT"), (std::vector<std::uint8_t>{0b10110001}));
    BOOST_CHECK_EQUAL(drv.getBytes("OUTPUT"), (std::vector<std::uint8_t>{0x56u, 0x78u, 0x9Au}));
    BOOST_CHECK_EQUAL(drv.getBytes("TESTARR"), (std::vector<std::uint8_t>{0xDE, 0xBC}));
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL"), 0xABCDu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_A"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_B"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_C"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_D"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_E"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_F"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_G"), 0);

    BOOST_CHECK(d.close());

    Device d2("{transfer_layer: [{name: intf2, type: FakeInterface}],"
               "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                             "init: {"
                                "FOOBAR: 0x313,"
                                "OUTPUT: [0x11, 0x22, 0x44],"
                                "TRIGGER: [0x1F, 0xB3],"
                                "TESTARR: [0x87, 0x4E],"
                                "TESTVAL: 0x91A2,"
                                "TESTVAL_A: 144434,"
                                "TESTVAL_B: 1465,"
                                "TESTVAL_C: 1465,"
                                "TESTVAL_D: 0x291083F,"
                                "TESTVAL_E: 0x36630C181,"
                                "TESTVAL_F: 0x3B9C70E0E0701FD,"
                                "TESTVAL_G: 0x7BCF1E1E0F03C07F,"
                                "NOT_A_REGISTER: 0x7BCF1E1E0F03C07F"    //Non-existent register, should be simply ignored
                             "}}],"
               "registers: []}");

    BOOST_REQUIRE(d2.init());

    RegisterDriver& drv2 = dynamic_cast<RegisterDriver&>(d2.driver("drv2"));

    BOOST_CHECK_EQUAL(drv2.getValue("VERSION"), 11);
    BOOST_CHECK_EQUAL(drv2.getValue("FOOBAR"), 787);
    BOOST_CHECK_EQUAL(drv2.getBytes("INPUT"), (std::vector<std::uint8_t>{0b10110001}));
    BOOST_CHECK_EQUAL(drv2.getBytes("OUTPUT"), (std::vector<std::uint8_t>{0x11, 0x22, 0x44}));
    BOOST_CHECK_EQUAL(drv2.getBytes("TESTARR"), (std::vector<std::uint8_t>{0x87, 0x4E}));
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL"), 0x91A2u);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_A"), 0x23432u);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_B"), 0b10110111001u);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_C"), 0b10110111001u);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_D"), 0x291083Fu);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_E"), 0x36630C181u);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_F"), 0x3B9C70E0E0701FDu);
    BOOST_CHECK_EQUAL(drv2.getValue("TESTVAL_G"), 0x7BCF1E1E0F03C07Fu);

    BOOST_CHECK(d2.close());
}

BOOST_AUTO_TEST_CASE(Test6_writeAndRead)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK_EQUAL(drv.getValue("FOOBAR"), 582);
    BOOST_CHECK_EQUAL(drv.getBytes("OUTPUT"), (std::vector<std::uint8_t>{0x56u, 0x78u, 0x9Au}));
    BOOST_CHECK_EQUAL(drv.getBytes("TESTARR"), (std::vector<std::uint8_t>{0xDE, 0xBC}));
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL"), 0xABCDu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_A"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_B"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_C"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_D"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_E"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_F"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_G"), 0);

    drv.setValue("FOOBAR", 1023);
    drv.setBytes("OUTPUT", std::vector<std::uint8_t>{0xFFu, 0xFFu, 0xFFu});
    drv.setBytes("TESTARR", std::vector<std::uint8_t>{0xFFu, 0xFFu});
    drv.setValue("TESTVAL", 0xFFFFu);
    drv.setValue("TESTVAL_A", 0x3FFFFu);
    drv.setValue("TESTVAL_B", 0x7FFu);
    drv.setValue("TESTVAL_C", 0x7FFu);
    drv.setValue("TESTVAL_D", 0x3FFFFFFu);
    drv.setValue("TESTVAL_E", 0x3FFFFFFFFu);
    drv.setValue("TESTVAL_F", 0x3FFFFFFFFFFFFFFu);
    drv.setValue("TESTVAL_G", 0x7FFFFFFFFFFFFFFFu);

    BOOST_CHECK_EQUAL(drv.getValue("FOOBAR"), 1023);
    BOOST_CHECK_EQUAL(drv.getBytes("OUTPUT"), (std::vector<std::uint8_t>{0xFFu, 0xFFu, 0xFFu}));
    BOOST_CHECK_EQUAL(drv.getBytes("TESTARR"), (std::vector<std::uint8_t>{0xFFu, 0xFFu}));
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL"), 0xFFFFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_A"), 0x3FFFFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_B"), 0x7FFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_C"), 0x7FFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_D"), 0x3FFFFFFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_E"), 0x3FFFFFFFFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_F"), 0x3FFFFFFFFFFFFFFu);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_G"), 0x7FFFFFFFFFFFFFFFu);

    drv.setValue("FOOBAR", 0);
    drv.setBytes("OUTPUT", std::vector<std::uint8_t>{0, 0, 0});
    drv.setBytes("TESTARR", std::vector<std::uint8_t>{0, 0});
    drv.setValue("TESTVAL", 0);
    drv.setValue("TESTVAL_A", 0);
    drv.setValue("TESTVAL_B", 0);
    drv.setValue("TESTVAL_C", 0);
    drv.setValue("TESTVAL_D", 0);
    drv.setValue("TESTVAL_E", 0);
    drv.setValue("TESTVAL_F", 0);
    drv.setValue("TESTVAL_G", 0);

    BOOST_CHECK_EQUAL(drv.getValue("FOOBAR"), 0);
    BOOST_CHECK_EQUAL(drv.getBytes("OUTPUT"), (std::vector<std::uint8_t>{0, 0, 0}));
    BOOST_CHECK_EQUAL(drv.getBytes("TESTARR"), (std::vector<std::uint8_t>{0, 0}));
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_A"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_B"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_C"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_D"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_E"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_F"), 0);
    BOOST_CHECK_EQUAL(drv.getValue("TESTVAL_G"), 0);
}

BOOST_AUTO_TEST_CASE(Test7_genericGetSet)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
             "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F, "
                           "init: {"
                              "TESTVAL_A: 144434,"
                              "TESTVAL_B: 1465,"
                              "TESTVAL_C: 1465,"
                              "TESTVAL_D: 0x291083F,"
                              "TESTVAL_E: 0x36630C181,"
                              "TESTVAL_F: 0x3B9C70E0E0701FD,"
                              "TESTVAL_G: 0x7BCF1E1E0F03C07F"
                           "}}],"
             "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("VERSION")), 11);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("FOOBAR")), 582);
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv.get("INPUT")), (std::vector<std::uint8_t>{0b10110001}));
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv.get("OUTPUT")), (std::vector<std::uint8_t>{0x56u, 0x78u, 0x9Au}));
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv.get("TESTARR")), (std::vector<std::uint8_t>{0xDE, 0xBC}));
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL")), 0xABCDu);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_A")), 0x23432u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_B")), 0b10110111001u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_C")), 0b10110111001u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_D")), 0x291083Fu);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_E")), 0x36630C181u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_F")), 0x3B9C70E0E0701FDu);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_G")), 0x7BCF1E1E0F03C07Fu);

    drv.set("FOOBAR", 129);
    drv.set("OUTPUT", std::vector<std::uint8_t>{0x23u, 0x55u, 0xAFu});
    drv.set("TESTARR", std::vector<std::uint8_t>{0xF5u, 0xA9u});
    drv.set("TESTVAL", 0x9752u);
    drv.set("TESTVAL_A", 0x3910Fu);
    drv.set("TESTVAL_B", 0x70Au);
    drv.set("TESTVAL_C", 0x148u);
    drv.set("TESTVAL_D", 0x1F0DD21u);
    drv.set("TESTVAL_E", 0x255F42EE0u);
    drv.set("TESTVAL_F", 0x1D186AB43D452E2u);
    drv.set("TESTVAL_G", 0x6EFCD1A87B1674F9u);

    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("FOOBAR")), 129);
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv.get("OUTPUT")), (std::vector<std::uint8_t>{0x23u, 0x55u, 0xAFu}));
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv.get("TESTARR")), (std::vector<std::uint8_t>{0xF5u, 0xA9u}));
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL")), 0x9752u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_A")), 0x3910Fu);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_B")), 0x70Au);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_C")), 0x148u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_D")), 0x1F0DD21u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_E")), 0x255F42EE0u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_F")), 0x1D186AB43D452E2u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv.get("TESTVAL_G")), 0x6EFCD1A87B1674F9u);
}

BOOST_AUTO_TEST_CASE(Test8_subscriptOperator)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
             "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F, "
                           "init: {"
                              "TESTVAL_A: 144434,"
                              "TESTVAL_B: 1465,"
                              "TESTVAL_C: 1465,"
                              "TESTVAL_D: 0x291083F,"
                              "TESTVAL_E: 0x36630C181,"
                              "TESTVAL_F: 0x3B9C70E0E0701FD,"
                              "TESTVAL_G: 0x7BCF1E1E0F03C07F"
                           "}}],"
             "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK_EQUAL(std::uint64_t{drv["VERSION"]}, 11);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["FOOBAR"]}, 582);
    BOOST_CHECK_EQUAL((std::vector<std::uint8_t>(drv["INPUT"])), (std::vector<std::uint8_t>{0b10110001}));
    BOOST_CHECK_EQUAL((std::vector<std::uint8_t>(drv["OUTPUT"])), (std::vector<std::uint8_t>{0x56u, 0x78u, 0x9Au}));
    BOOST_CHECK_EQUAL((std::vector<std::uint8_t>(drv["TESTARR"])), (std::vector<std::uint8_t>{0xDE, 0xBC}));
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL"]}, 0xABCDu);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_A"]}, 0x23432u);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_B"]}, 0b10110111001u);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_C"]}, 0b10110111001u);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_D"]}, 0x291083Fu);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_E"]}, 0x36630C181u);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_F"]}, 0x3B9C70E0E0701FDu);
    BOOST_CHECK_EQUAL(std::uint64_t{drv["TESTVAL_G"]}, 0x7BCF1E1E0F03C07Fu);

    drv["FOOBAR"] = 129;
    drv["OUTPUT"] = std::vector<std::uint8_t>{0x23u, 0x55u, 0xAFu};
    drv["TESTARR"] = std::vector<std::uint8_t>{0xF5u, 0xA9u};
    drv["TESTVAL"] = 0x9752u;
    drv["TESTVAL_A"] = 0x3910Fu;
    drv["TESTVAL_B"] = 0x70Au;
    drv["TESTVAL_C"] = 0x148u;
    drv["TESTVAL_D"] = 0x1F0DD21u;
    drv["TESTVAL_E"] = 0x255F42EE0u;
    drv["TESTVAL_F"] = 0x1D186AB43D452E2u;
    drv["TESTVAL_G"] = 0x6EFCD1A87B1674F9u;

    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["FOOBAR"].get()), 129);
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv["OUTPUT"].get()), (std::vector<std::uint8_t>{0x23u, 0x55u, 0xAFu}));
    BOOST_CHECK_EQUAL(std::get<std::vector<std::uint8_t>>(drv["TESTARR"].get()), (std::vector<std::uint8_t>{0xF5u, 0xA9u}));
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL"].get()), 0x9752u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_A"].get()), 0x3910Fu);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_B"].get()), 0x70Au);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_C"].get()), 0x148u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_D"].get()), 0x1F0DD21u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_E"].get()), 0x255F42EE0u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_F"].get()), 0x1D186AB43D452E2u);
    BOOST_CHECK_EQUAL(std::get<std::uint64_t>(drv["TESTVAL_G"].get()), 0x6EFCD1A87B1674F9u);

    const auto& rstReg = drv["RESET"];
    rstReg = 0;

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(Test9_triggerWO)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    drv.trigger("RESET");
    drv.trigger("TRIGGER");

    Device d2("{transfer_layer: [{name: intf2, type: FakeInterface}],"
               "hw_drivers: [{name: drv2, type: TestRegDriver, interface: intf2, base_addr: 0x135F, "
                             "init: {"
                                "RESET: 0xBC,"
                                "TRIGGER: [0x1F, 0xB3]"
                             "}}],"
               "registers: []}");

    BOOST_REQUIRE(d2.init());

    RegisterDriver& drv2 = dynamic_cast<RegisterDriver&>(d2.driver("drv2"));

    drv2.trigger("RESET");
    drv2.trigger("TRIGGER");

    BOOST_CHECK(true);

    const auto& rstReg = drv["RESET"];
    rstReg.trigger();

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(Test10_typeModeSizeMismatch)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    int exceptionCtr = 0;

    //Data type mismatch

    try { (void)drv.getBytes("FOOBAR"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.setBytes("FOOBAR", {0x12u, 0x34u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)drv.getValue("INPUT"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.setValue("OUTPUT", 0); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set("OUTPUT", 0); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set("FOOBAR", {0x12u, 0x34u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Access mode mismatch

    try { drv.setBytes("INPUT", std::vector<std::uint8_t>{0xABu}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.setValue("VERSION", 0x04u); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set("VERSION", 0x04u); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set("INPUT", std::vector<std::uint8_t>{0x12}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.trigger("VERSION"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Size mismatch

    try { drv.setBytes("OUTPUT", {0x12u, 0x34u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set("OUTPUT", {0x12u, 0x34u, 0x45u, 0x67u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 13);
}

BOOST_AUTO_TEST_CASE(Test11_wrongRegisterName)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x135F}],"
              "registers: []}");

    BOOST_REQUIRE(d.init());

    RegisterDriver& drv = dynamic_cast<RegisterDriver&>(d.driver("drv"));

    const std::string_view wrongRegName = "SOME_MISSING_REG_NAME";

    int exceptionCtr = 0;

    try { (void)drv.getBytes(wrongRegName); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.setBytes(wrongRegName, {}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)drv.getValue(wrongRegName); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.setValue(wrongRegName, 0); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)drv.get(wrongRegName); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set(wrongRegName, 0); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.set(wrongRegName, {}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { drv.trigger(wrongRegName); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 8);
}

BOOST_AUTO_TEST_CASE(Test12_testRegisterName)
{
    Device d("{transfer_layer: [{name: intf, type: FakeInterface}],"
              "hw_drivers: [{name: drv, type: TestRegDriver, interface: intf, base_addr: 0x0}],"
              "registers: []}");

    const RegisterDriver& drv = dynamic_cast<const RegisterDriver&>(d.driver("drv"));

    BOOST_CHECK(drv.testRegisterName("RESET"));
    BOOST_CHECK(drv.testRegisterName("VERSION"));
    BOOST_CHECK(drv.testRegisterName("FOOBAR"));
    BOOST_CHECK(drv.testRegisterName("INPUT"));
    BOOST_CHECK(drv.testRegisterName("OUTPUT"));
    BOOST_CHECK(drv.testRegisterName("TRIGGER"));
    BOOST_CHECK(drv.testRegisterName("TESTARR"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_A"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_B"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_C"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_D"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_E"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_F"));
    BOOST_CHECK(drv.testRegisterName("TESTVAL_G"));

    int exceptionCtr = 0;

    try { (void)drv.testRegisterName("SOME_MISSING_REG_NAME"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)drv.testRegisterName("otherMissingReg"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 2);
}

BOOST_AUTO_TEST_CASE(Test13_validRegisterNames)
{
    std::string testChars;
    testChars.push_back('_');
    for (int i = 0; i < 26; ++i)
        testChars.push_back('A'+i);

    BOOST_CHECK(RegisterDriver::isValidRegisterName(testChars));

    for (int i = 0; i < 128; ++i)
    {
        const char c = i;

        if (c == '\0')
            continue;

        if (testChars.find(c) != testChars.npos)
            continue;

        std::string testName = "TEST_REG_NAME_";
        testName.push_back(c);

        BOOST_CHECK(RegisterDriver::isValidRegisterName(testName) == false);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
