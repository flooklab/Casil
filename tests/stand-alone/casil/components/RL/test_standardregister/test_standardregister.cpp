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

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/RL/standardregister.h>

#include <boost/dynamic_bitset.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

using casil::Device;
using casil::RL::StandardRegister;

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
    using casil::Bytes::bitsetFromBytes;
    using casil::Bytes::composeByteVec;
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

BOOST_AUTO_TEST_CASE(Test8_advancedSelect)
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

    reg.root()(6, 3) = reg.root()[{3, 4, 5, 6}].toBits();
    //TODO replace by
    //reg.root()(6, 3) = reg.root()(3, 6).toBits();
    //if/once reverse selection gets implemented

    BOOST_CHECK_EQUAL((reg.root().toUInt()), 0b101100111u);

    int exceptionCtr = 0;

    try { (void)reg.root()(1, 2); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()(9, 1); }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[std::vector<std::size_t>{}]; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[{9, 1, 0}]; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { (void)reg.root()[{0, 1, 1}]; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 5);
}

BOOST_AUTO_TEST_CASE(Test9_zeroSize)
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

BOOST_AUTO_TEST_CASE(Test10_fieldConfExceptions)
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

BOOST_AUTO_TEST_CASE(Test11_otherExceptions)
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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
