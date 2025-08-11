/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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
#include <casil/logger.h>

#include <sstream>
#include <string>

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(ContextualLogger_Tests)

BOOST_AUTO_TEST_CASE(Test1_context)
{
    std::ostringstream logOutputStrm;

    using casil::Logger;
    Logger::addOutput(logOutputStrm);
    Logger::setLogLevel(Logger::LogLevel::DebugDebug);

    casil::Device exampleDev("{transfer_layer: [{name: intf, type: DummyInterface}],"
                              "hw_drivers: [{name: someDriverName, type: Logging-Driver, interface: intf}],"
                              "registers: []}");

    BOOST_REQUIRE(exampleDev.init());

    Logger::setLogLevel(Logger::LogLevel::Critical);

    BOOST_REQUIRE(exampleDev.close());

    Logger::removeOutput(logOutputStrm);

    const std::string testStr = logOutputStrm.str();

    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (1)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (2)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (3)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (4)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (5)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (6)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (7)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (8)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (9)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (10)") != testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not a regular log but just a test message! (11)") == testStr.npos);

    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not an error but yet another a test message! (0)") == testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not an error but yet another a test message! (1)") == testStr.npos);
    BOOST_CHECK(testStr.find("HL/Logging-Driver/\"someDriverName\": This is not an error but yet another a test message! (2)") != testStr.npos);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
