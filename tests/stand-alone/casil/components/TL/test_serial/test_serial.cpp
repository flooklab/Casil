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

BOOST_AUTO_TEST_SUITE(Serial_Tests)

BOOST_AUTO_TEST_CASE(Test1_portSettings)
{
    //Valid sets of init options
    const std::vector<std::string> optsStrs1 = {", baudrate: 9600, read_termination: \"\"",
                                                ", baudrate: 1, read_termination: \"\\n\"",
                                                ", baudrate: 1, read_termination: 123",
                                                ", baudrate: 9600, read_termination: \"\\n\", write_termination: \"\\r\", "
                                                    "bytesize: 8, parity: E, stopbits: \"1.5\", flow_ctrl: S",
                                                ", baudrate: 9600, read_termination: \"\\n\", stopbits: 2, flow_ctrl: H",
                                                ", baudrate: 9600, read_termination: \"\\n\", flow_ctrl: H, dsrdtr: false",
                                                ", baudrate: 9600, read_termination: \"\\n\", dsrdtr: false",
                                                ", baudrate: 9600, read_termination: \"\\n\", xonxoff: false, rtscts: false",
                                                ", baudrate: 9600, read_termination: \"\\n\", xonxoff: true, rtscts: false",
                                                ", baudrate: 9600, read_termination: \"\\n\", xonxoff: false, rtscts: true"};

    int numOptsOk = 0;

    for (const auto& optsStr : optsStrs1)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: Serial, init: {port: \"INVALID\"" + optsStr + "}}],"
                      "hw_drivers: [], registers: []}");
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
    const std::vector<std::string> optsStrs2 = {", read_termination: \"\"",
                                                ", baudrate: 9600",
                                                ", baudrate: 0, read_termination: \"\"",
                                                ", baudrate: abc, read_termination: \"\"",
                                                ", baudrate: 9600, read_termination: \"\\n\", bytesize: 4",
                                                ", baudrate: 9600, read_termination: \"\\n\", parity: M",
                                                ", baudrate: 9600, read_termination: \"\\n\", parity: \"FooBar\"",
                                                ", baudrate: 9600, read_termination: \"\\n\", stopbits: 1.50",
                                                ", baudrate: 9600, read_termination: \"\\n\", stopbits: \"1.7\"",
                                                ", baudrate: 9600, read_termination: \"\\n\", flow_ctrl: x",
                                                ", baudrate: 9600, read_termination: \"\\n\", flow_ctrl: \"None\"",
                                                ", baudrate: 9600, read_termination: \"\\n\", dsrdtr: true",
                                                ", baudrate: 9600, read_termination: \"\\n\", flow_ctrl: H, dsrdtr: true",
                                                ", baudrate: 9600, read_termination: \"\\n\", flow_ctrl: H, xonxoff: false, rtscts: false",
                                                ", baudrate: 9600, read_termination: \"\\n\", xonxoff: true, rtscts: true"};

    int numOptsErr = 0;

    for (const auto& optsStr : optsStrs2)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: Serial, init: {port: \"INVALID\"" + optsStr + "}}],"
                      "hw_drivers: [], registers: []}");
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
