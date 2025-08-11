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

#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/HL/Muxed/gpio.h>

#include <boost/dynamic_bitset.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using casil::Device;
using casil::HL::GPIO;

namespace boost { using casil::Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(GPIO_Tests)

BOOST_AUTO_TEST_CASE(Test1_zeroSize)
{
    try
    {
        Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
                  "hw_drivers: [{name: drv, type: GPIO, interface: intf, base_addr: 0x0, size: 0}],"
                  "registers: []}");
        (void)d;

        BOOST_CHECK(false);
    }
    catch (const std::runtime_error&)
    {
        BOOST_CHECK(true);
    }
}

BOOST_AUTO_TEST_CASE(Test2_initFail)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: drv, type: GPIO, interface: intf, base_addr: 0x0, size: 8}],"
              "registers: []}");

    BOOST_CHECK(!d.init());     //Cannot determine FPGA version with dummy interface
}

BOOST_AUTO_TEST_CASE(Test3_getSize)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: drv1, type: GPIO, interface: intf, base_addr: 0x000, size: 7},"
                           "{name: drv2, type: GPIO, interface: intf, base_addr: 0x100, size: 8},"
                           "{name: drv3, type: GPIO, interface: intf, base_addr: 0x200, size: 14},"
                           "{name: drv4, type: GPIO, interface: intf, base_addr: 0x300, size: 24}],"
              "registers: []}");

    const GPIO& drv1 = dynamic_cast<const GPIO&>(d.driver("drv1"));
    const GPIO& drv2 = dynamic_cast<const GPIO&>(d.driver("drv2"));
    const GPIO& drv3 = dynamic_cast<const GPIO&>(d.driver("drv3"));
    const GPIO& drv4 = dynamic_cast<const GPIO&>(d.driver("drv4"));

    BOOST_CHECK_EQUAL(drv1.getSize(), 7);
    BOOST_CHECK_EQUAL(drv2.getSize(), 8);
    BOOST_CHECK_EQUAL(drv3.getSize(), 14);
    BOOST_CHECK_EQUAL(drv4.getSize(), 24);
}

BOOST_AUTO_TEST_CASE(Test4_bitsetFromBytes)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: drv1, type: GPIO, interface: intf, base_addr: 0x000, size: 7},"
                           "{name: drv2, type: GPIO, interface: intf, base_addr: 0x100, size: 8},"
                           "{name: drv3, type: GPIO, interface: intf, base_addr: 0x200, size: 14},"
                           "{name: drv4, type: GPIO, interface: intf, base_addr: 0x300, size: 24}],"
              "registers: []}");

    const GPIO& drv1 = dynamic_cast<const GPIO&>(d.driver("drv1"));
    const GPIO& drv2 = dynamic_cast<const GPIO&>(d.driver("drv2"));
    const GPIO& drv3 = dynamic_cast<const GPIO&>(d.driver("drv3"));
    const GPIO& drv4 = dynamic_cast<const GPIO&>(d.driver("drv4"));

    BOOST_CHECK_EQUAL(drv1.bitsetFromBytes(std::vector<std::uint8_t>{0b1010101u}), boost::dynamic_bitset(std::string("1010101")));
    BOOST_CHECK_EQUAL(drv1.bitsetFromBytes(std::vector<std::uint8_t>{0b1100111u}), boost::dynamic_bitset(std::string("1100111")));

    BOOST_CHECK_EQUAL(drv2.bitsetFromBytes(std::vector<std::uint8_t>{0b11010101u}), boost::dynamic_bitset(std::string("11010101")));
    BOOST_CHECK_EQUAL(drv2.bitsetFromBytes(std::vector<std::uint8_t>{0b11001111u}), boost::dynamic_bitset(std::string("11001111")));

    BOOST_CHECK_EQUAL(drv3.bitsetFromBytes(std::vector<std::uint8_t>{0b101011, 0b10001001}),
                      boost::dynamic_bitset(std::string("10101110001001")));
    BOOST_CHECK_EQUAL(drv3.bitsetFromBytes(std::vector<std::uint8_t>{0b111100, 0b00100011}),
                      boost::dynamic_bitset(std::string("11110000100011")));

    BOOST_CHECK_EQUAL(drv4.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001}),
                      boost::dynamic_bitset(std::string("111110111011011010001001")));
    BOOST_CHECK_EQUAL(drv4.bitsetFromBytes(std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}),
                      boost::dynamic_bitset(std::string("000100100101101110100010")));

    int exceptionCtr = 0;

    try { auto x = drv1.bitsetFromBytes(std::vector<std::uint8_t>{}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv1.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv2.bitsetFromBytes(std::vector<std::uint8_t>{}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv2.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv3.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv3.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv4.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv4.bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001, 0b10001001}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 8);
}

BOOST_AUTO_TEST_CASE(Test5_bytesFromBitset)
{
    Device d("{transfer_layer: [{name: intf, type: DummyMuxedInterface}],"
              "hw_drivers: [{name: drv1, type: GPIO, interface: intf, base_addr: 0x000, size: 7},"
                           "{name: drv2, type: GPIO, interface: intf, base_addr: 0x100, size: 8},"
                           "{name: drv3, type: GPIO, interface: intf, base_addr: 0x200, size: 14},"
                           "{name: drv4, type: GPIO, interface: intf, base_addr: 0x300, size: 24}],"
              "registers: []}");

    const GPIO& drv1 = dynamic_cast<const GPIO&>(d.driver("drv1"));
    const GPIO& drv2 = dynamic_cast<const GPIO&>(d.driver("drv2"));
    const GPIO& drv3 = dynamic_cast<const GPIO&>(d.driver("drv3"));
    const GPIO& drv4 = dynamic_cast<const GPIO&>(d.driver("drv4"));

    BOOST_CHECK_EQUAL(drv1.bytesFromBitset(boost::dynamic_bitset(std::string("1010101"))), (std::vector<std::uint8_t>{0b1010101u}));
    BOOST_CHECK_EQUAL(drv1.bytesFromBitset(boost::dynamic_bitset(std::string("1100111"))), (std::vector<std::uint8_t>{0b1100111u}));

    BOOST_CHECK_EQUAL(drv2.bytesFromBitset(boost::dynamic_bitset(std::string("11010101"))), (std::vector<std::uint8_t>{0b11010101u}));
    BOOST_CHECK_EQUAL(drv2.bytesFromBitset(boost::dynamic_bitset(std::string("11001111"))), (std::vector<std::uint8_t>{0b11001111u}));

    BOOST_CHECK_EQUAL(drv3.bytesFromBitset(boost::dynamic_bitset(std::string("10101110001001"))),
                      (std::vector<std::uint8_t>{0b101011, 0b10001001}));
    BOOST_CHECK_EQUAL(drv3.bytesFromBitset(boost::dynamic_bitset(std::string("11110000100011"))),
                      (std::vector<std::uint8_t>{0b111100, 0b00100011}));

    BOOST_CHECK_EQUAL(drv4.bytesFromBitset(boost::dynamic_bitset(std::string("111110111011011010001001"))),
                      (std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001}));
    BOOST_CHECK_EQUAL(drv4.bytesFromBitset(boost::dynamic_bitset(std::string("000100100101101110100010"))),
                      (std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}));

    int exceptionCtr = 0;

    try { auto x = drv1.bytesFromBitset(boost::dynamic_bitset(std::string("100010"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv1.bytesFromBitset(boost::dynamic_bitset(std::string("10001011"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv2.bytesFromBitset(boost::dynamic_bitset(std::string("1000100"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv2.bytesFromBitset(boost::dynamic_bitset(std::string("100010011"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv3.bytesFromBitset(boost::dynamic_bitset(std::string("1111101110110"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv3.bytesFromBitset(boost::dynamic_bitset(std::string("111110111011011"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv4.bytesFromBitset(boost::dynamic_bitset(std::string("11111011101101101000100"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { auto x = drv4.bytesFromBitset(boost::dynamic_bitset(std::string("1111101110110110100010011"))); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 8);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
