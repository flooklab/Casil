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
#include <casil/HL/Direct/scpi.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

using casil::Device;
using casil::HL::SCPI;

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(SCPI_Tests)

BOOST_AUTO_TEST_CASE(Test1_initFail)
{
    Device d("{transfer_layer: [{name: intf, type: DummyInterface}],"
              "hw_drivers: [{name: drv, type: SCPI, interface: intf, init: {device: \"Keithley 2400\"}}],"
              "registers: []}");

    //Expecting device identifier mismatch because of DummyInterface
    BOOST_CHECK(d.init() == false);
    BOOST_CHECK(d["intf"].init() == true);
    BOOST_CHECK(d["drv"].init() == false);
    BOOST_CHECK(d["drv"].init(true) == false);

    BOOST_CHECK(d.close() == true);
}

BOOST_AUTO_TEST_CASE(Test2_commandsTopLevelOnly)
{
    Device d("{transfer_layer: [{name: intf, type: DummyInterface}],"
              "hw_drivers: [{name: drv, type: SCPI, interface: intf, init: {device: \"Keithley 2400\"}}],"
              "registers: []}");

    const SCPI& scpi = dynamic_cast<SCPI&>(d["drv"]);

    //Command calls that should work

    try
    {
        scpi.writeCommand("off");
        scpi.writeCommand("off", std::nullopt);
        scpi.writeCommand("off", -1);
        scpi.writeCommand("off", std::nullopt, {});
        scpi.writeCommand("off", std::nullopt, 123);
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        scpi.writeCommand("set_voltage", std::nullopt, "0");
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        //Expecting empty return value because of DummyInterface
        BOOST_CHECK(scpi.queryCommand("get_current") == "");
        BOOST_CHECK(scpi.queryCommand("get_current", std::nullopt) == "");
        BOOST_CHECK(scpi.queryCommand("get_current", -1) == "");
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        BOOST_CHECK(scpi.command("get_current").has_value() == true);
        BOOST_CHECK(scpi.command("get_current", std::nullopt, 123).has_value() == true);
        BOOST_CHECK(scpi.command("off").has_value() == false);

        BOOST_CHECK(scpi("get_current").has_value() == true);
        BOOST_CHECK(scpi("get_current", std::nullopt, 123).has_value() == true);
        BOOST_CHECK(scpi("off").has_value() == false);
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    //Command calls that should not work

    int exceptionCtr = 0;

    //Wrong channel
    try { scpi("off", 0); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Wrong channel
    try { scpi("off", 1); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 2);
    exceptionCtr = 0;

    //Missing value argument
    try { scpi.writeCommand("set_voltage"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Missing value argument
    try { scpi.writeCommand("set_voltage", std::nullopt); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Missing value argument
    try { scpi.writeCommand("set_voltage", -1); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Missing value argument
    try { scpi.writeCommand("set_voltage", std::nullopt, {}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 4);
}

BOOST_AUTO_TEST_CASE(Test3_commandsWithChannels)
{
    Device d("{transfer_layer: [{name: intf, type: DummyInterface}],"
              "hw_drivers: [{name: drv, type: SCPI, interface: intf, init: {device: TTi QL355TP}}],"
              "registers: []}");

    const SCPI& scpi = dynamic_cast<SCPI&>(d["drv"]);

    //Command calls that should work

    try
    {
        scpi.writeCommand("off");
        scpi.writeCommand("off", std::nullopt);
        scpi.writeCommand("off", -1);
        scpi.writeCommand("off", std::nullopt, {});
        scpi.writeCommand("off", std::nullopt, 123);

        scpi.writeCommand("off", 1);
        scpi.writeCommand("off", 2);
        scpi.writeCommand("off", 3);
        scpi.writeCommand("off", 3, 123.4);
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        scpi.writeCommand("set_voltage", 1, "0.0");
        scpi.writeCommand("set_voltage", 2, 0.0);
        scpi.writeCommand("set_voltage", 3, 0);
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        //Expecting empty return value because of DummyInterface
        BOOST_CHECK(scpi.queryCommand("get_current", 1) == "");
        BOOST_CHECK(scpi.queryCommand("get_current", 2) == "");
        BOOST_CHECK(scpi.queryCommand("get_current", 3) == "");

        BOOST_CHECK(scpi.queryCommand("get_current_limit", 1) == "");
        BOOST_CHECK(scpi.queryCommand("get_current_limit", 2) == "");
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    try
    {
        BOOST_CHECK(scpi.command("get_current", 1).has_value() == true);
        BOOST_CHECK(scpi.command("get_current", 2).has_value() == true);
        BOOST_CHECK(scpi.command("get_current", 3).has_value() == true);
        BOOST_CHECK(scpi.command("get_current", 3, 123).has_value() == true);
        BOOST_CHECK(scpi.command("off").has_value() == false);
        BOOST_CHECK(scpi.command("off", 1).has_value() == false);
        BOOST_CHECK(scpi.command("off", 2).has_value() == false);
        BOOST_CHECK(scpi.command("off", 3).has_value() == false);

        BOOST_CHECK(scpi("get_current", 1).has_value() == true);
        BOOST_CHECK(scpi("get_current", 2).has_value() == true);
        BOOST_CHECK(scpi("get_current", 3).has_value() == true);
        BOOST_CHECK(scpi("get_current", 3, 123).has_value() == true);
        BOOST_CHECK(scpi("off").has_value() == false);
        BOOST_CHECK(scpi("off", 1).has_value() == false);
        BOOST_CHECK(scpi("off", 2).has_value() == false);
        BOOST_CHECK(scpi("off", 3).has_value() == false);
    }
    catch (const std::invalid_argument&)
    {
        BOOST_CHECK(false);
    }

    //Command calls that should not work

    int exceptionCtr = 0;

    //Wrong channel
    try { scpi("get_current_limit"); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Wrong channel
    try { scpi("get_current_limit", 3); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 2);
    exceptionCtr = 0;

    //Missing value argument
    try { scpi.writeCommand("set_voltage", 1, {}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
