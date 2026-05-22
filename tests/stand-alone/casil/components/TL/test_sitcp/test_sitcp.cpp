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

#include <casil/device.h>

#include <stdexcept>
#include <string>
#include <vector>

using casil::Device;

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(SiTCP_Tests)

BOOST_AUTO_TEST_CASE(Test1_configValues)
{
    //Valid sets of init options
    const std::vector<std::string> optsStrs1 = {"ip: 127.0.0.1, udp_port: 10355",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 10354, tcp_connection: false, tcp_to_bus: false, "
                                                    "connect_timeout: 3.9",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 0, tcp_connection: false, tcp_to_bus: false, "
                                                    "connect_timeout: 0",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 10354, tcp_connection: true, tcp_to_bus: false",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 10354, tcp_connection: true, tcp_to_bus: true"};

    int numOptsOk = 0;

    for (const auto& optsStr : optsStrs1)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: SiTCP, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
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
    const std::vector<std::string> optsStrs2 = {"ip: \"\", udp_port: 10355",
                                                "ip: 127.0.0.1, udp_port: 0",
                                                "udp_port: 10355",
                                                "ip: 127.0.0.1",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_connection: true",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 0, tcp_connection: true",
                                                "ip: 127.0.0.1, udp_port: 10355, tcp_port: 10354, tcp_connection: false, tcp_to_bus: true",
                                                "ip: 127.0.0.1, udp_port: 10355, connect_timeout: -1.2"};

    int numOptsErr = 0;

    for (const auto& optsStr : optsStrs2)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: SiTCP, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            ++numOptsErr;    //Increment error counter on error
        }
    }

    BOOST_CHECK_EQUAL(numOptsErr, optsStrs2.size());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
