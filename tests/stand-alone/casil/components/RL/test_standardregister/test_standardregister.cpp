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

#include "testreadbackdriver.h"

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/RL/standardregister.h>

#include <boost/dynamic_bitset.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using casil::Device;
using casil::RL::StandardRegister;

namespace Bytes = casil::Bytes;

namespace boost { using Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(StandardRegister_Tests)

BOOST_AUTO_TEST_CASE(Test1_noFields)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 3}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 3, "
                           "fields: []}"
                         "]}");

    BOOST_REQUIRE(d.reg("reg").init());

    Device d2("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 3}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 3}]}");
    (void)d2;

    BOOST_CHECK(d.reg("reg").init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(reg[2]), false);
    BOOST_CHECK_EQUAL(reg[1].get(), false);
    BOOST_CHECK_EQUAL(reg[0].get(), false);

    reg[2] = true;
    reg[1] = true;

    BOOST_CHECK_EQUAL(reg[2].get(), true);
    BOOST_CHECK_EQUAL(static_cast<bool>(reg[2]), true);
    BOOST_CHECK_EQUAL(reg[1].get(), true);
    BOOST_CHECK_EQUAL(reg[0].get(), false);
}

BOOST_AUTO_TEST_CASE(Test2_flatLayout)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 11}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 11, "
                           "fields: ["
                            "{name: Reg1, offset: 10, size: 6},"
                            "{name: Reg2, offset: 2, size: 3}"
                           "]}"
                         "]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg.getSize(), 11);
    BOOST_CHECK_EQUAL(reg.root().getSize(), 11);
    BOOST_CHECK_EQUAL(reg.root().getOffset(), 10);
    BOOST_CHECK_EQUAL(reg.root().getTotalOffset(), 10);
    BOOST_CHECK_EQUAL(reg["Reg1"].getSize(), 6);
    BOOST_CHECK_EQUAL(reg["Reg1"].getOffset(), 10);
    BOOST_CHECK_EQUAL(reg["Reg1"].getTotalOffset(), 10);
    BOOST_CHECK_EQUAL(reg["Reg2"].getSize(), 3);
    BOOST_CHECK_EQUAL(reg["Reg2"].getOffset(), 2);
    BOOST_CHECK_EQUAL(reg["Reg2"].getTotalOffset(), 2);

    BOOST_CHECK_EQUAL(reg[10].get(), false);
    BOOST_CHECK_EQUAL(reg[9].get(), false);
    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg[7].get(), false);
    BOOST_CHECK_EQUAL(reg[6].get(), false);
    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg[4].get(), false);
    BOOST_CHECK_EQUAL(reg[3].get(), false);
    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(reg[1].get(), false);
    BOOST_CHECK_EQUAL(reg[0].get(), false);

    BOOST_CHECK_EQUAL(reg["Reg1"][5].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][4].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][3].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][2].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][2].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][0].get(), false);

    reg[9] = true;
    reg[4] = true;
    reg[3] = true;
    reg["Reg2"][0] = true;

    BOOST_CHECK_EQUAL(reg[10].get(), false);
    BOOST_CHECK_EQUAL(reg[9].get(), true);
    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg[7].get(), false);
    BOOST_CHECK_EQUAL(reg[6].get(), false);
    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg[4].get(), true);
    BOOST_CHECK_EQUAL(reg[3].get(), true);
    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(reg[1].get(), false);
    BOOST_CHECK_EQUAL(reg[0].get(), true);

    BOOST_CHECK_EQUAL(reg["Reg1"][5].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][4].get(), true);
    BOOST_CHECK_EQUAL(reg["Reg1"][3].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][2].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg1"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][2].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["Reg2"][0].get(), true);
}

BOOST_AUTO_TEST_CASE(Test3_subFields)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 9}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 9, fields: ["
                                "{name: COMP1, offset: 8, size: 2, fields: ["
                                    "{name: R0, size: 1, offset: 1},"
                                    "{name: L0, size: 1, offset: 0}"
                                "]},"
                                "{name: COMP2, offset: 5, size: 6, fields : ["
                                    "{name: En0, size: 1, offset: 3},"
                                    "{name: En1, size: 1, offset: 2},"
                                    "{name: CTR, size: 2, offset: 1}"
                                "]}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1.R0"][0].get(), false);

    BOOST_CHECK_EQUAL(reg[6].get(), false);

    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2"][5].get(), false);

    BOOST_CHECK_EQUAL(reg[0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"][0].get(), false);

    reg[8] = true;
    reg[6] = true;

    BOOST_CHECK_EQUAL(reg[8].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1"][1].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1.R0"][0].get(), true);

    BOOST_CHECK_EQUAL(reg[6].get(), true);

    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2"][5].get(), false);

    BOOST_CHECK_EQUAL(reg[0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"][0].get(), false);

    reg["COMP1"][1] = false;
    reg["COMP2"][5] = true;
    reg["COMP2"][3] = true;

    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1.R0"][0].get(), false);

    BOOST_CHECK_EQUAL(reg[6].get(), true);

    BOOST_CHECK_EQUAL(reg[5].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP2"][5].get(), true);

    BOOST_CHECK_EQUAL(reg[0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"][0].get(), false);

    reg["COMP2.CTR"][1] = true;
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"][0].get(), false);

    reg["COMP2.CTR"][0] = true;

    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1.R0"][0].get(), false);

    BOOST_CHECK_EQUAL(reg[6].get(), true);

    BOOST_CHECK_EQUAL(reg[5].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP2"][5].get(), true);

    BOOST_CHECK_EQUAL(reg[0].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP2"][0].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"][0].get(), true);
}

BOOST_AUTO_TEST_CASE(Test4_nestedSubFields)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 10}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 10, fields: ["
                                "{name: SomeRegister, offset: 9, size: 10, fields : ["
                                    "{name: Conf0, size: 5, offset: 9, fields : ["
                                        "{name: Thr, size: 3, offset: 4},"
                                        "{name: Bias, size: 2, offset: 1}"
                                    "]},"
                                    "{name: Conf1, size: 5, offset: 4, fields : ["
                                        "{name: Thr, size: 3, offset: 4},"
                                        "{name: Bias, size: 2, offset: 1}"
                                    "]}"
                                "]}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg["SomeRegister"].getSize(), 10);
    BOOST_CHECK_EQUAL(reg["SomeRegister"].getOffset(), 9);
    BOOST_CHECK_EQUAL(reg["SomeRegister"].getTotalOffset(), 9);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"].getSize(), 5);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"].getOffset(), 9);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"].getTotalOffset(), 9);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"].getSize(), 3);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"].getOffset(), 4);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"].getTotalOffset(), 9);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Bias"].getSize(), 2);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Bias"].getOffset(), 1);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Bias"].getTotalOffset(), 6);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1"].getSize(), 5);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1"].getOffset(), 4);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1"].getTotalOffset(), 4);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"].getSize(), 3);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"].getOffset(), 4);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"].getTotalOffset(), 4);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Bias"].getSize(), 2);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Bias"].getOffset(), 1);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Bias"].getTotalOffset(), 1);

    reg["SomeRegister.Conf0.Thr"][2] = true;
    reg["SomeRegister.Conf0.Thr"][1] = false;
    reg["SomeRegister.Conf0.Thr"][0] = true;
    reg["SomeRegister.Conf0.Bias"][1] = false;
    reg["SomeRegister.Conf0.Bias"][0] = true;

    BOOST_CHECK_EQUAL(reg[9].get(), true);
    BOOST_CHECK_EQUAL(reg[8].get(), false);
    BOOST_CHECK_EQUAL(reg[7].get(), true);
    BOOST_CHECK_EQUAL(reg[6].get(), false);
    BOOST_CHECK_EQUAL(reg[5].get(), true);
    BOOST_CHECK_EQUAL(reg[4].get(), false);
    BOOST_CHECK_EQUAL(reg[3].get(), false);
    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(reg[1].get(), false);
    BOOST_CHECK_EQUAL(reg[0].get(), false);

    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"][4].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"][3].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"][2].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"][1].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0"][0].get(), true);

    reg[9] = false;
    reg[8] = true;
    reg[7] = false;
    reg[6] = true;
    reg[5] = false;
    reg["SomeRegister.Conf1"][4] = true;
    reg["SomeRegister.Conf1"][3] = true;
    reg["SomeRegister.Conf1"][2] = false;
    reg["SomeRegister.Conf1"][1] = true;
    reg["SomeRegister.Conf1"][0] = true;

    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"][2].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"][1].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Thr"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Bias"][1].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf0.Bias"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"][2].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"][1].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Thr"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Bias"][1].get(), true);
    BOOST_CHECK_EQUAL(reg["SomeRegister.Conf1.Bias"][0].get(), true);
}

BOOST_AUTO_TEST_CASE(Test5_uintAssignConvert)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 11}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 11, "
                           "fields: ["
                            "{name: Reg1, offset: 10, size: 6},"
                            "{name: Reg2, offset: 2, size: 3}"
                           "]}"
                         "]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg.root().toUInt(), std::uint64_t{0x0000u});

    reg.root() = std::uint64_t{0xFFFFu};

    BOOST_CHECK_EQUAL(reg.root().toUInt(), std::uint64_t{0x07FFu});

    reg[10] = false;
    reg[9] = true;
    reg[8] = false;
    reg[7] = false;
    reg[6] = true;
    reg[5] = true;
    reg[4] = false;
    reg[3] = false;
    reg[2] = false;
    reg[1] = true;
    reg[0] = true;

    BOOST_CHECK_EQUAL(reg.root().toUInt(), std::uint64_t{0x0263u});

    reg.root() = std::uint64_t{0x051Au};

    BOOST_CHECK_EQUAL(reg[10].get(), true);
    BOOST_CHECK_EQUAL(reg[9].get(), false);
    BOOST_CHECK_EQUAL(reg[8].get(), true);
    BOOST_CHECK_EQUAL(reg[7].get(), false);
    BOOST_CHECK_EQUAL(reg[6].get(), false);
    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg[4].get(), true);
    BOOST_CHECK_EQUAL(reg[3].get(), true);
    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(reg[1].get(), true);
    BOOST_CHECK_EQUAL(reg[0].get(), false);

    BOOST_CHECK_EQUAL(reg.root().toUInt(), std::uint64_t{0x051Au});

    BOOST_CHECK_EQUAL(reg["Reg1"].toUInt(), std::uint64_t{0x0028u});
    BOOST_CHECK_EQUAL(reg["Reg2"].toUInt(), std::uint64_t{0x0002u});

    reg[4] = false;
    reg["Reg1"] = std::uint64_t{0b011011u};
    reg["Reg2"] = std::uint64_t{0b101u};

    BOOST_CHECK_EQUAL(reg["Reg1"].toUInt(), std::uint64_t{0x001Bu});
    BOOST_CHECK_EQUAL(reg["Reg2"].toUInt(), std::uint64_t{0x0005u});
    BOOST_CHECK_EQUAL(reg.root().toUInt(), std::uint64_t{0x036Du});

    BOOST_CHECK_EQUAL(static_cast<std::uint64_t>(reg["Reg1"]), std::uint64_t{0x001Bu});
    BOOST_CHECK_EQUAL(static_cast<std::uint64_t>(reg["Reg2"]), std::uint64_t{0x0005u});
    BOOST_CHECK_EQUAL(static_cast<std::uint64_t>(reg.root()), std::uint64_t{0x036Du});
}

BOOST_AUTO_TEST_CASE(Test6_bitsetAssignConvert)
{
    using Bytes::bitsetFromBytes;
    using Bytes::composeByteVec;
    using boost::dynamic_bitset;

    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 11}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 11, "
                           "fields: ["
                            "{name: Reg1, offset: 10, size: 6},"
                            "{name: Reg2, offset: 2, size: 3}"
                           "]}"
                         "]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg.root().toBits(), dynamic_bitset<>(11));

    reg.root() = bitsetFromBytes(composeByteVec(true, 0xFFFFu), 11);

    BOOST_CHECK_EQUAL(reg.root().toBits(), bitsetFromBytes(composeByteVec(true, 0x07FFu), 11));

    reg[10] = false;
    reg[9] = true;
    reg[8] = false;
    reg[7] = false;
    reg[6] = true;
    reg[5] = true;
    reg[4] = false;
    reg[3] = false;
    reg[2] = false;
    reg[1] = true;
    reg[0] = true;

    BOOST_CHECK_EQUAL(reg.root().toBits(), bitsetFromBytes(composeByteVec(true, 0x0263u), 11));

    reg.root() = bitsetFromBytes(composeByteVec(true, 0x051Au), 11);

    BOOST_CHECK_EQUAL(reg[10].get(), true);
    BOOST_CHECK_EQUAL(reg[9].get(), false);
    BOOST_CHECK_EQUAL(reg[8].get(), true);
    BOOST_CHECK_EQUAL(reg[7].get(), false);
    BOOST_CHECK_EQUAL(reg[6].get(), false);
    BOOST_CHECK_EQUAL(reg[5].get(), false);
    BOOST_CHECK_EQUAL(reg[4].get(), true);
    BOOST_CHECK_EQUAL(reg[3].get(), true);
    BOOST_CHECK_EQUAL(reg[2].get(), false);
    BOOST_CHECK_EQUAL(reg[1].get(), true);
    BOOST_CHECK_EQUAL(reg[0].get(), false);

    BOOST_CHECK_EQUAL(reg.root().toBits(), bitsetFromBytes(composeByteVec(true, 0x051Au), 11));

    BOOST_CHECK_EQUAL(reg["Reg1"].toBits(), bitsetFromBytes(composeByteVec(true, 0x0028u), 6));
    BOOST_CHECK_EQUAL(reg["Reg2"].toBits(), bitsetFromBytes(composeByteVec(true, 0x0002u), 3));

    reg[4] = false;
    reg["Reg1"] = bitsetFromBytes(composeByteVec(true, 0b011011u), 6);
    reg["Reg2"] = bitsetFromBytes(composeByteVec(true, 0b101u), 3);

    BOOST_CHECK_EQUAL(reg["Reg1"].toBits(), bitsetFromBytes(composeByteVec(true, 0x001Bu), 6));
    BOOST_CHECK_EQUAL(reg["Reg2"].toBits(), bitsetFromBytes(composeByteVec(true, 0x0005u), 3));
    BOOST_CHECK_EQUAL(reg.root().toBits(), bitsetFromBytes(composeByteVec(true, 0x036Du), 11));

    BOOST_CHECK_EQUAL(static_cast<dynamic_bitset<>>(reg["Reg1"]), bitsetFromBytes(composeByteVec(true, 0x001Bu), 6));
    BOOST_CHECK_EQUAL(static_cast<dynamic_bitset<>>(reg["Reg2"]), bitsetFromBytes(composeByteVec(true, 0x0005u), 3));
    BOOST_CHECK_EQUAL(static_cast<dynamic_bitset<>>(reg.root()), bitsetFromBytes(composeByteVec(true, 0x036Du), 11));
}

BOOST_AUTO_TEST_CASE(Test7_repeat)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 12}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 12, fields: ["
                                "{name: COMP1, offset: 11, size: 2, repeat: 3, fields: ["
                                    "{name: R0, size: 1, offset: 1},"
                                    "{name: L0, size: 1, offset: 0}"
                                "]},"
                                "{name: COMP2, offset: 5, size: 6, fields : ["
                                    "{name: En0, size: 1, offset: 3},"
                                    "{name: En1, size: 1, offset: 2},"
                                    "{name: CTR, size: 2, offset: 1}"
                                "]}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg["COMP1"].getSize(), 6);
    BOOST_CHECK_EQUAL(reg["COMP1"].getOffset(), 11);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0).getSize(), 2);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0).getOffset(), 5);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1).getSize(), 2);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1).getOffset(), 3);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(2).getSize(), 2);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(2).getOffset(), 1);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)["R0"].getSize(), 1);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)["R0"].getOffset(), 1);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1)["R0"].getSize(), 1);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1)["R0"].getOffset(), 1);

    BOOST_CHECK_EQUAL(reg["COMP1"].getTotalOffset(), 11);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0).getTotalOffset(), 11);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)["R0"].getTotalOffset(), 11);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)["L0"].getTotalOffset(), 10);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1).getTotalOffset(), 9);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1)["R0"].getTotalOffset(), 9);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1)["L0"].getTotalOffset(), 8);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(2).getTotalOffset(), 7);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(2)["R0"].getTotalOffset(), 7);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(2)["L0"].getTotalOffset(), 6);
    BOOST_CHECK_EQUAL(reg["COMP2"].getTotalOffset(), 5);
    BOOST_CHECK_EQUAL(reg["COMP2.En0"].getTotalOffset(), 3);
    BOOST_CHECK_EQUAL(reg["COMP2.En1"].getTotalOffset(), 2);
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"].getTotalOffset(), 1);

    reg[11] = true;

    BOOST_CHECK_EQUAL(reg["COMP1"][5].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)["R0"][0].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(0)[1].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1"]["#0"]["R0"][0].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1.#0.R0"][0].get(), true);
    BOOST_CHECK_EQUAL(reg["COMP1"].n(1)["R0"][0].get(), false);
    BOOST_CHECK_EQUAL(reg["COMP1.#2"]["R0"][0].get(), false);

    int exceptionCtr = 0;

    //COMP2 has no repetitions
    try { (void)reg["COMP2"].n(0)[0].get(); }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    //COMP1 has only 3 repetitions
    try { (void)reg["COMP1"].n(3)["R0"][0].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //COMP1 has only 3 repetitions
    try { (void)reg["COMP1"]["#5"]["R0"][0].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Must use one of the repetitions of COMP1
    try { (void)reg["COMP1"]["R0"][0].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Should not throw, as no repetitions for COMP2
    try { (void)reg["COMP2"]["En0"][0].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 4);
}

BOOST_AUTO_TEST_CASE(Test8_initApplyDefaults)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 28}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 28, fields: ["
                                "{name: COMP0, offset: 27, size: 4, repeat: 2, fields: ["
                                    "{name: R0, size: 2, offset: 3},"
                                    "{name: L0, size: 2, offset: 1}"
                                "]},"
                                "{name: COMP1, offset: 19, size: 4, repeat: 3, fields: ["
                                    "{name: R0, size: 2, offset: 3},"
                                    "{name: L0, size: 2, offset: 1}"
                                "]},"
                                "{name: COMP2, offset: 7, size: 8, fields : ["
                                    "{name: En0, size: 1, offset: 5},"
                                    "{name: En1, size: 1, offset: 4},"
                                    "{name: CTR, size: 4, offset: 3}"
                                "]}"
                            "], init: {"
                                    "COMP0.#0: 0x9,"
                                    "COMP1.#0.R0: 3,"
                                    "COMP1.#2.R0: 0,"
                                    "COMP1.#2.L0: 0x2,"
                                    "COMP2.En0: 0,"
                                    "COMP2.CTR: \"0b1010\""
                            "}}]}");

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL(reg["COMP0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.En0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.En1"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"].toUInt(), std::uint64_t{0x0u});

    BOOST_TEST(reg.init());

    BOOST_CHECK_EQUAL(reg["COMP0"].toUInt(), std::uint64_t{0x90u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.R0"].toUInt(), std::uint64_t{0b10u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.L0"].toUInt(), std::uint64_t{0b01u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.R0"].toUInt(), std::uint64_t{0x3u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.L0"].toUInt(), std::uint64_t{0x2u});
    BOOST_CHECK_EQUAL(reg["COMP2.En0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.En1"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"].toUInt(), std::uint64_t{0b1010u});

    reg["COMP0"] = 0b11101111u;
    reg["COMP1.#0.R0"] = 0b01u;
    reg["COMP1.#0.L0"] = 0b10u;
    reg["COMP1.#1.R0"] = 0b01u;
    reg["COMP1.#1.L0"] = 0b00u;
    reg["COMP1.#2.R0"] = 0b11u;
    reg["COMP1.#2.L0"] = 0b01u;
    reg["COMP2.En0"] = 0b1u;
    reg["COMP2.En1"] = 0b1u;
    reg["COMP2.CTR"] = 0b00011u;

    BOOST_CHECK_EQUAL(reg["COMP0.#0.R0"].toUInt(), std::uint64_t{0b11u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.L0"].toUInt(), std::uint64_t{0b10u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.R0"].toUInt(), std::uint64_t{0b11u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.L0"].toUInt(), std::uint64_t{0b11u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.R0"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.L0"].toUInt(), std::uint64_t{0x2u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.R0"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.R0"].toUInt(), std::uint64_t{0x3u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.L0"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP2.En0"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP2.En1"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"].toUInt(), std::uint64_t{0b0011u});

    reg.applyDefaults();

    BOOST_CHECK_EQUAL(reg["COMP0.#0.R0"].toUInt(), std::uint64_t{0b10u});
    BOOST_CHECK_EQUAL(reg["COMP0.#0.L0"].toUInt(), std::uint64_t{0b01u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.R0"].toUInt(), std::uint64_t{0b11u});
    BOOST_CHECK_EQUAL(reg["COMP0.#1.L0"].toUInt(), std::uint64_t{0b11u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.R0"].toUInt(), std::uint64_t{0x3u});
    BOOST_CHECK_EQUAL(reg["COMP1.#0.L0"].toUInt(), std::uint64_t{0x2u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.R0"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP1.#1.L0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.R0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP1.#2.L0"].toUInt(), std::uint64_t{0x2u});
    BOOST_CHECK_EQUAL(reg["COMP2.En0"].toUInt(), std::uint64_t{0x0u});
    BOOST_CHECK_EQUAL(reg["COMP2.En1"].toUInt(), std::uint64_t{0x1u});
    BOOST_CHECK_EQUAL(reg["COMP2.CTR"].toUInt(), std::uint64_t{0b1010u});

    //Test exceptions

    int exceptionCtr = 0;

    const std::string confStr1 = "{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                                  "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 10}],"
                                  "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 10, fields: ["
                                                    "{name: MainField, offset: 9, size: 10}"
                                                "], init: ";
    const std::string confStr2 = "}]}";

    try { Device d2(confStr1 + "{ MainField: 12345 }" + confStr2); (void)d2; }              //OK
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d2(confStr1 + "{ MainField: \"0b0011010101\" }" + confStr2); (void)d2; }   //OK
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d2(confStr1 + "{ MainField: \"10b0011010101\" }" + confStr2); (void)d2; }  //Invalid prefix
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d2(confStr1 + "{ MainField: \"0b101010110\" }" + confStr2); (void)d2; }    //Too short
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d2(confStr1 + "{ MainField: \"0b10011010101\" }" + confStr2); (void)d2; }  //Too long
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d2(confStr1 + "{ MainField: \"0b00110abc01\" }" + confStr2); (void)d2; }   //Invalid characters
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 4);
}

BOOST_AUTO_TEST_CASE(Test9_bitOrder)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 109}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 109, fields: ["
                                "{name: COL0, offset: 108, size: 15, fields: ["
                                    "{name: Trim1, offset: 14, size: 5},"
                                    "{name: Trim2, offset: 9, size: 5, bit_order: [4, 3, 2, 1, 0]},"
                                    "{name: Trim3, offset: 4, size: 5, bit_order: [0, 1, 3, 4, 2]}"
                                "]},"
                                "{name: ROW, offset: 93, size: 11, repeat: 8, bit_order: [8, 9, 10, 4, 5, 6, 7, 0, 1, 2, 3], fields: ["
                                    "{name: En0, offset: 10, size: 1},"
                                    "{name: En1, offset: 9, size: 1},"
                                    "{name: En2, offset: 8, size: 1},"
                                    "{name: InL, offset: 7, size: 4, bit_order: [0, 1, 2, 3]},"
                                    "{name: InR, offset: 3, size: 4}"
                                "]},"
                                "{name: Test, offset: 5, size: 6, bit_order: [5, 3, 1, 4, 2, 0], fields: ["
                                    "{name: Upper, offset: 5, size: 3},"
                                    "{name: Lower, offset: 2, size: 3, bit_order: [1, 0, 2]}"
                                "]}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    reg["COL0"] = 0b10110'10110'10110u;
    reg["ROW"] = boost::dynamic_bitset(std::string("11010101010" "11010101010" "11010101010" "11010101010"
                                                   "11010101010" "11010101010" "11010101010" "11010101010"));
    reg["Test"] = 0b110100u;

    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string(
                          "10110" "10110" "10110"
                          "11010101010" "11010101010" "11010101010" "11010101010" "11010101010" "11010101010" "11010101010" "11010101010"
                          "111000"
                          )));
    BOOST_CHECK_EQUAL(reg["ROW"].n(0).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(1).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(2).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(3).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(4).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(5).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(6).toBits(), boost::dynamic_bitset(std::string("01101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(7).toUInt(),                                  0b01101010101u);

    reg.root() = boost::dynamic_bitset(std::string(
                   "01001" "01001" "01001"
                   "00101010101" "00101010101" "00101010101" "00101010101" "00101010101" "00101010101" "00101010101" "00101010101"
                   "000111"
                    ));

    BOOST_CHECK_EQUAL(reg["COL0"].toUInt(), 0b01001'01001'01001u);
    BOOST_CHECK_EQUAL(reg["ROW"].toBits(), boost::dynamic_bitset(std::string("00101010101" "00101010101" "00101010101" "00101010101"
                                                                             "00101010101" "00101010101" "00101010101" "00101010101")));
    BOOST_CHECK_EQUAL(reg["ROW"].n(7).toUInt(),                             0b10010101010u);
    BOOST_CHECK_EQUAL(reg["Test"].toUInt(), 0b001011u);

    BOOST_CHECK_EQUAL(reg["COL0"]["Trim1"].toUInt(), 0b01001u);
    BOOST_CHECK_EQUAL(reg["COL0"]["Trim2"].toUInt(), 0b01001u);
    BOOST_CHECK_EQUAL(reg["COL0"]["Trim3"].toUInt(), 0b10100u);

    BOOST_CHECK_EQUAL(reg["ROW"].n(7)["En0"].toUInt(), 0b1u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(7)["En1"].toUInt(), 0b0u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(7)["En2"].toUInt(), 0b0u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(7)["InL"].toUInt(), 0b0101u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(7)["InR"].toUInt(), 0b1010u);

    BOOST_CHECK_EQUAL(reg["Test.Upper"].toUInt(), 0b001u);
    BOOST_CHECK_EQUAL(reg["Test.Lower"].toUInt(), 0b110u);

    reg["COL0"]["Trim2"] = 0b11010u;
    reg["COL0"]["Trim3"] = 0b01010u;
    reg["ROW"].n(1)["InL"] = 0b1110u;
    reg["Test.Lower"] = 0b010u;

    BOOST_CHECK_EQUAL(reg["COL0"]["Trim2"].toUInt(), 0b11010u);
    BOOST_CHECK_EQUAL(reg["COL0"]["Trim3"].toUInt(), 0b01010u);
    BOOST_CHECK_EQUAL(reg["COL0"].toUInt(), 0b01001'11010'10010u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(1)["InL"].toUInt(), 0b1110u);
    BOOST_CHECK_EQUAL(reg["ROW"].n(1).toUInt(), 0b10001111010u);
    BOOST_CHECK_EQUAL(reg["Test.Lower"].toUInt(), 0b010u);
    BOOST_CHECK_EQUAL(reg["Test"].toUInt(), 0b001001u);

    //Test exceptions

    int exceptionCtr = 0;

    const std::string confStr1 = "{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                                  "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 12}],"
                                  "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 12, fields: ["
                                    "{name: Test2, offset: 11, size: 4, repeat: 2, fields: ["
                                        "{name: Inner, offset: 3, size: 4, bit_order: ";

    const std::string confStr2 = "}]},"
                                    "{name: Test1, offset: 3, size: 4, fields: ["
                                        "{name: Inner, offset: 3, size: 4, bit_order: ";

    const std::string confStr3 = "}]}]}]}";

    for (const auto& bitOrderStr : std::vector<std::string>{"[3, 1, 2, 0]",     //OK
                                                            "[0, 1, 2, 3]",     //OK
                                                            "[0, 0, 2, 3]",     //Duplicate bit
                                                            "[0, 1, 4, 3]",     //Index out of range
                                                            "[0, 1, 2, 3, 4]",  //Wrong size (and index out of range)
                                                            "[0, 1, 2]"})       //Wrong size
    {
        try { Device d2(confStr1 + bitOrderStr + confStr2 + bitOrderStr + confStr3); (void)d2; }
        catch (const std::runtime_error&) { ++exceptionCtr; }
    }

    BOOST_CHECK_EQUAL(exceptionCtr, 4);
}

BOOST_AUTO_TEST_CASE(Test10_advancedSelect)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 9}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 9, fields: ["
                                "{name: COMP1, offset: 8, size: 2},"
                                "{name: COMP2, offset: 5, size: 6}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_CHECK_EQUAL((reg.root()(8, 0).toUInt()), 0u);
    BOOST_CHECK_EQUAL((reg.root()[{1,3,5}].toUInt()), 0u);

    reg.root()[{1, 3, 5, 0}] = 0b1100u;

    BOOST_CHECK_EQUAL((reg.root()[{1, 3, 5}].toUInt()), 0b110u);
    BOOST_CHECK_EQUAL((reg.root().toUInt()), 0b000001010u);

    reg.root()[0] = true;
    reg.root()(4, 1) = 0x0u;

    BOOST_CHECK_EQUAL((reg.root().toUInt()), 0b000000001u);

    reg.root() = 0b110010111u;

    BOOST_CHECK_EQUAL((reg.root()(7, 2).toUInt()), 0b100101u);
    BOOST_CHECK_EQUAL(reg["COMP1"].toUInt(), 0b11u);
    BOOST_CHECK_EQUAL(reg["COMP2"].toUInt(), 0b010111u);
    BOOST_CHECK_EQUAL(reg["COMP2"](4, 2).toUInt(), 0b101u);

    reg["COMP2"](3, 3)[0] = true;
    reg["COMP1"][{0, 1}][1] = false;
    reg["COMP1"][{0, 1}][0] = true;

    BOOST_CHECK_EQUAL((reg.root().toUInt()), 0b100011111u);

    reg.root()(6, 3) = reg.root()(3, 6).toBits();   //Reverse some bits

    BOOST_CHECK_EQUAL((reg.root().toUInt()), 0b101100111u);

    int exceptionCtr = 0;

    try { (void)reg.root()[{7, 1, 0}]; }                        //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()(2, 1); }                             //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()(1, 2); }                             //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()(9, 1); }                             //Most significant bit out of range
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()(1, 9); }                             //Least significant bit out of range
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[std::vector<std::size_t>{}]; }       //Set is empty
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[{9, 1, 0}]; }                        //Index out of range
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[{0, 1, 1}]; }                        //Duplicate index
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 5);
}

BOOST_AUTO_TEST_CASE(Test11_zeroSize)
{
    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 3}],"
                  "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 0}]}");
        (void)d;

        BOOST_CHECK(false);
    }
    catch (const std::runtime_error&)
    {
        BOOST_CHECK(true);
    }
}

BOOST_AUTO_TEST_CASE(Test12_fieldConfExceptions)
{
    int exceptionCtr = 0;

    const std::string confStr1 = "{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                                 "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 12}],"
                                 "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 12, ";
    const std::string confStr2 = "}]}";

    try { Device d(confStr1 + "fields: 2" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: []" + confStr2); (void)d; }      //OK
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [1, 2]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 2, unknownKey: 7}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{offset: 11, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, offset: 11}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: [1, 2, 3], offset: 11, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: \"\", offset: 11, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, offset: number, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 0}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 2, repeat: 0}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: \"#COMP1\", offset: 11, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { Device d(confStr1 + "fields: [{name: \"CO.MP1\", offset: 11, size: 2}]" + confStr2); (void)d; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        Device d(confStr1 + "fields: ["
                                "{name: COMP1, offset: 11, size: 2, repeat: 3, fields: ["
                                    "{name: R0, size: 1, offset: 1},"
                                    "{name: R0, size: 1, offset: 0}"
                                "]}"
                            "]" + confStr2);
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 12, fields: ["
                                        "{name: R0, size: 2, offset: 11, repeat: 7}"
                                    "]}"
                            "]" + confStr2);
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 12, fields: ["
                                        "{name: R0, size: 13, offset: 11}"
                                    "]}"
                            "]" + confStr2);
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try
    {
        Device d(confStr1 + "fields: [{name: COMP1, offset: 11, size: 12, fields: ["
                                        "{name: R0, size: 2, offset: 15}"
                                    "]}"
                            "]" + confStr2);
        (void)d;
    }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 17);
}

BOOST_AUTO_TEST_CASE(Test13_otherExceptions)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 9}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 9, fields: ["
                                "{name: COMP1, offset: 8, size: 2, fields: ["
                                    "{name: R0, size: 1, offset: 1},"
                                    "{name: L0, size: 1, offset: 0}"
                                "]},"
                                "{name: COMP2, offset: 5, size: 6, fields : ["
                                    "{name: En0, size: 1, offset: 3},"
                                    "{name: En1, size: 1, offset: 2},"
                                    "{name: CTR, size: 2, offset: 1}"
                                "]}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    int exceptionCtr = 0;

    //Wrong path

    try { (void)reg["COMP3"].getSize(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg["COMP2"]["En0"].getSize(); }                //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg["COMP2"]["Enable"].getSize(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Index range

    try { (void)reg[0].get(); }                                 //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg[9].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg["COMP2"][0].get(); }                        //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg["COMP2"][6].get(); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    //Assignment bitset length

    try { reg["COMP2"] = boost::dynamic_bitset(std::string("010101")); }    //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { reg["COMP2"] = boost::dynamic_bitset(std::string("0101011")); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { reg["COMP2"] = boost::dynamic_bitset(std::string("01010")); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 6);

    exceptionCtr = 0;

    using BoolRef = StandardRegister::BoolRef;
    using RegField = StandardRegister::RegField;

    boost::dynamic_bitset bits(std::string("111111"));

    try { RegField field1(bits, "SomeName", 5, 5); (void)field1; }      //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field1(bits, "SomeName", 0, 5); (void)field1; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field1(bits, "SomeName", 5, 3); (void)field1; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field1(bits, "SomeName", 5, 6); (void)field1; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 3);

    exceptionCtr = 0;

    RegField field1(bits, "SomeName", 5, 5);

    try { RegField field2(field1, "SomeName", 4, 4); (void)field2; }    //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field2(field1, "SomeName", 0, 4); (void)field2; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field2(field1, "SomeName", 4, 2); (void)field2; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { RegField field2(field1, "SomeName", 4, 5); (void)field2; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 3);

    exceptionCtr = 0;

    try { BoolRef bref(bits, 0); (void)bref; }                  //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { BoolRef bref(bits, 6); (void)bref; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { BoolRef bref(field1, 0); (void)bref; }                //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { BoolRef bref(field1, 5); (void)bref; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 2);
}

BOOST_AUTO_TEST_CASE(Test14_getSetFunctions)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 9}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 9, fields: ["
                                "{name: some_field, offset: 8, size: 9}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    BOOST_REQUIRE_EQUAL(reg.root().toUInt(), 0u);
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("000000000")));

    reg.root() = 0x16Eu;

    BOOST_REQUIRE_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("101101110")));
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("101101110")));

    reg.set(boost::dynamic_bitset(std::string("000111011")));

    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("000111011")));
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("000111011")));

    reg.set(0x1C2u);

    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("111000010")));
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("111000010")));

    reg.setAll(true);

    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("111111111")));
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("111111111")));

    reg.setAll(false);

    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("000000000")));
    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("000000000")));

    reg.root().setAll(true);
    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("111111111")));
    reg.root().setAll(false);
    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("000000000")));

    reg.root().set(0x1C2u);
    BOOST_CHECK_EQUAL(reg.root().toBits(), boost::dynamic_bitset(std::string("111000010")));

    reg.root().set(boost::dynamic_bitset(std::string("101101110")));
    BOOST_CHECK_EQUAL(reg.root().toUInt(), 0x16Eu);
}

BOOST_AUTO_TEST_CASE(Test15_toFromBytes)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 11}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 11},"
                          "{name: reg2, type: StandardRegister, hw_driver: GPIO, size: 11, lsb_side_padding: False}]}");

    BOOST_REQUIRE(d["reg"].init());
    BOOST_REQUIRE(d["reg2"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));
    StandardRegister& reg2 = dynamic_cast<StandardRegister&>(d.reg("reg2"));

    reg.fromBytes({0b10110111u, 0b00100000u});
    reg2.fromBytes({0b00000101u, 0b10111001u});

    BOOST_CHECK_EQUAL(reg.get(), boost::dynamic_bitset(std::string("10110111001")));
    BOOST_CHECK_EQUAL((reg.toBytes()), (std::vector<std::uint8_t>{0b10110111u, 0b00100000u}));

    BOOST_CHECK_EQUAL(reg2.get(), boost::dynamic_bitset(std::string("10110111001")));
    BOOST_CHECK_EQUAL((reg2.toBytes()), (std::vector<std::uint8_t>{0b00000101u, 0b10111001u}));

    reg.set(boost::dynamic_bitset(std::string("01001000110")));
    reg2.set(boost::dynamic_bitset(std::string("01001000110")));

    BOOST_CHECK_EQUAL((reg.toBytes()), (std::vector<std::uint8_t>{0b01001000u, 0b11000000u}));
    BOOST_CHECK_EQUAL((reg2.toBytes()), (std::vector<std::uint8_t>{0b00000010u, 0b01000110u}));
}

BOOST_AUTO_TEST_CASE(Test16_writeReadAndReadTree)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: TestReadbackDriver, interface: intf, base_addr: 0x0, size: 11},"
                           "{name: GPIO2, type: TestReadbackDriver, interface: intf, base_addr: 0x0, size: 11},"
                           "{name: GPIO3, type: TestReadbackDriver, interface: intf, base_addr: 0x0, size: 11}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 11},"
                          "{name: reg2, type: StandardRegister, hw_driver: GPIO2, size: 11, lsb_side_padding: False},"
                          "{name: reg3, type: StandardRegister, hw_driver: GPIO3, size: 11, auto_start: True}]}");

    BOOST_REQUIRE(d.init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));
    StandardRegister& reg2 = dynamic_cast<StandardRegister&>(d.reg("reg2"));
    StandardRegister& reg3 = dynamic_cast<StandardRegister&>(d.reg("reg3"));

    reg.set(boost::dynamic_bitset(std::string("10110111001")));
    reg2.set(boost::dynamic_bitset(std::string("10110111001")));
    reg3.set(boost::dynamic_bitset(std::string("10110111001")));

    reg.write();
    reg2.write();
    reg3.write();

    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("00000000000")));
    BOOST_CHECK_EQUAL(reg2.getRead(), boost::dynamic_bitset(std::string("00000000000")));
    BOOST_CHECK_EQUAL(reg3.getRead(), boost::dynamic_bitset(std::string("00000000000")));

    BOOST_CHECK_EQUAL(reg.rootRead().toBits(), boost::dynamic_bitset(std::string("00000000000")));

    reg.read();
    reg2.read();
    reg3.read();

    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("10110111001")));
    BOOST_CHECK_EQUAL(reg2.getRead(), boost::dynamic_bitset(std::string("10110111001")));
    BOOST_CHECK_EQUAL(reg3.getRead(), boost::dynamic_bitset(std::string("11111111111")));

    BOOST_CHECK_EQUAL(reg.rootRead().toBits(), boost::dynamic_bitset(std::string("10110111001")));

    dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO"]).exec();

    BOOST_CHECK_EQUAL(dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO"]).isDone(), true);
    BOOST_CHECK_EQUAL(dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO2"]).isDone(), false);
    BOOST_CHECK_EQUAL(dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO3"]).isDone(), true);

    reg.read();
    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("11111111111")));

    reg.rootRead() = 0x2D6u;
    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("01011010110")));

    reg.set(boost::dynamic_bitset(std::string("10010111101")));
    reg.write(1);

    BOOST_CHECK_EQUAL((dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO"]).getData(1)), (std::vector<std::uint8_t>{0b10010111u}));
    bool wrongSizeThrown = false;
    try { (void)dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO"]).getData(2); }
    catch (const std::invalid_argument&) { wrongSizeThrown = true; }
    BOOST_CHECK(wrongSizeThrown);

    reg.read(1);

    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("10010111000")));

    reg.set(boost::dynamic_bitset(std::string("10010111101")));
    reg.write();
    reg.read(1);

    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("10010111000")));

    reg.read(2);

    BOOST_CHECK_EQUAL(reg.getRead(), boost::dynamic_bitset(std::string("10010111101")));

    //Test exceptions

    int exceptionCtr = 0;

    try { (void)reg.read(3); }                                  //Larger than register byte size
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.read(); }                                   //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    dynamic_cast<casil::Layers::HL::TestReadbackDriver&>(d["GPIO"]).setData({0b10101010u, 0b10101010u});    //Make read calls fail

    try { (void)reg.read(); }                                   //Fails with std::runtime_error due to wrong size returned from driver
    catch (const std::invalid_argument&) { ++exceptionCtr; }
    catch (const std::runtime_error&) { ++exceptionCtr; }

    try { (void)reg.write(1); }                                 //OK
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.write(3); }                                 //Larger than register byte size
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 3);
}

BOOST_AUTO_TEST_CASE(Test17_loadDumpConf)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: GPIO, type: GPIO, interface: intf, base_addr: 0x0, size: 9}],"
              "registers: [{name: reg, type: StandardRegister, hw_driver: GPIO, size: 9, fields: ["
                                "{name: some_field, offset: 8, size: 9}"
                            "]}]}");

    BOOST_REQUIRE(d["reg"].init());

    StandardRegister& reg = dynamic_cast<StandardRegister&>(d.reg("reg"));

    //TODO
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
