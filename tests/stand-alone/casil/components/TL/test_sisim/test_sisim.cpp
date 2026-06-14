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

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/TL/Muxed/sisim.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

using casil::Device;
using casil::TL::SiSim;

namespace Bytes = casil::Bytes;

namespace boost { using Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(SiSim_Tests)

BOOST_AUTO_TEST_CASE(Test1_configValues)
{
    //Valid sets of init options
    const std::vector<std::string> optsStrs1 = {"host: localhost, port: 10354, connect_retries: 0",
                                                "",
                                                "port: fooBar",             //Results in default
                                                "connect_retries: abc",     //Results in default
                                                "connect_retries: -123"};   //Results in default

    int numOptsOk = 0;

    for (const auto& optsStr : optsStrs1)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: SiSim, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            continue;   //Skip incrementing success counter on error
        }
        ++numOptsOk;
    }

    BOOST_CHECK_EQUAL(numOptsOk, optsStrs1.size());

    //Invalid sets of init options
    const std::vector<std::string> optsStrs2 = {"host: \"\"",
                                                "port: 0",
                                                "port: 65536"};

    int numOptsErr = 0;

    for (const auto& optsStr : optsStrs2)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: SiSim, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            ++numOptsErr;    //Increment error counter on error
        }
    }

    BOOST_CHECK_EQUAL(numOptsErr, optsStrs2.size());
}

BOOST_AUTO_TEST_CASE(Test2_messageCreateParse)
{
    BOOST_CHECK(SiSim::parseMessageType(0x00u) == SiSim::MessageType::WriteRequest);
    BOOST_CHECK(SiSim::parseMessageType(0x01u) == SiSim::MessageType::ReadRequest);
    BOOST_CHECK(SiSim::parseMessageType(0x02u) == SiSim::MessageType::ReadResponse);

    BOOST_CHECK_EQUAL(SiSim::parseMessageSize(std::vector<std::uint8_t>{0x1Fu, 0x2Du, 0x8Au, 0x23u}), 0x1F2D8A23u);

    BOOST_CHECK_EQUAL((SiSim::createWriteRequest(0x12F89A23B7856D45u, std::vector<std::uint8_t>{0xABu, 0xCDu, 0x00u})),
                      (std::vector<std::uint8_t>{0, 0x0, 0x0, 0x0, 0xBu, 0x12u, 0xF8u, 0x9Au, 0x23u,
                                                 0xB7u, 0x85u, 0x6Du, 0x45u, 0xABu, 0xCDu, 0x00u}));

    BOOST_CHECK_EQUAL((SiSim::createReadRequest(0x01F89A23B7856D57u, 0xFF345678u)),
                      (std::vector<std::uint8_t>{1, 0x0, 0x0, 0x0, 0xCu, 0x01u, 0xF8u, 0x9Au, 0x23u,
                                                 0xB7u, 0x85u, 0x6Du, 0x57u, 0xFFu, 0x34u, 0x56u, 0x78u}));

    BOOST_CHECK_EQUAL((SiSim::createReadResponse(std::vector<std::uint8_t>{0xF9u, 0xCDu, 0x10u})),
                      (std::vector<std::uint8_t>{2, 0x0, 0x0, 0x0, 0x3u, 0xF9u, 0xCDu, 0x10u}));

    const auto wrReq = SiSim::createWriteRequest(0x12F89A23B7856D45u, std::vector<std::uint8_t>{0xABu, 0xCDu, 0x00u});
    const auto parsedWrReq = SiSim::parseWriteRequest(std::vector<std::uint8_t>(wrReq.begin()+5, wrReq.end()));

    BOOST_CHECK_EQUAL(std::get<0>(parsedWrReq), 0x12F89A23B7856D45u);
    BOOST_CHECK_EQUAL((std::get<1>(parsedWrReq)), (std::vector<std::uint8_t>{0xABu, 0xCDu, 0x00u}));

    const auto rdReq = SiSim::createReadRequest(0x01F89A23B7856D57u, 0xFF345678u);
    const auto parsedRdReq = SiSim::parseReadRequest(std::vector<std::uint8_t>(rdReq.begin()+5, rdReq.end()));

    BOOST_CHECK_EQUAL(std::get<0>(parsedRdReq), 0x01F89A23B7856D57u);
    BOOST_CHECK_EQUAL(std::get<1>(parsedRdReq), 0xFF345678u);

    const auto rdResp = SiSim::createReadResponse(std::vector<std::uint8_t>{0xF9u, 0xCDu, 0x10u});
    const auto parsedRdResp = SiSim::parseReadResponse(std::vector<std::uint8_t>(rdResp.begin()+5, rdResp.end()));

    BOOST_CHECK_EQUAL((parsedRdResp), (std::vector<std::uint8_t>{0xF9u, 0xCDu, 0x10u}));
}

BOOST_AUTO_TEST_CASE(Test3_messageCreateParseExceptions)
{
    int exceptionCtr = 0;

    try { (void)SiSim::parseMessageType(0x03u); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)SiSim::parseMessageSize(std::vector<std::uint8_t>{0x1Fu, 0x2Du, 0x8Au, 0x23u, 0xDDu}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)SiSim::parseMessageSize(std::vector<std::uint8_t>{0x1Fu, 0x2Du, 0x8Au}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)SiSim::parseWriteRequest(std::vector<std::uint8_t>{0x12u, 0xF8u, 0x9Au, 0x23u, 0xB7u, 0x85u, 0x6Du}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)SiSim::parseReadRequest(std::vector<std::uint8_t>{0x12u, 0xF8u, 0x9Au, 0x23u, 0xB7u, 0x85u, 0x6Du,
                                                                  0x45u, 0xABu, 0xCDu, 0x00u, 0x4Eu, 0x32u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)SiSim::parseReadRequest(std::vector<std::uint8_t>{0x12u, 0xF8u, 0x9Au, 0x23u, 0xB7u, 0x85u, 0x6Du,
                                                                  0x45u, 0xABu, 0xCDu, 0x00u}); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 6);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
