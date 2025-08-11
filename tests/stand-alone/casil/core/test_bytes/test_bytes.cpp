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

#include <boost/dynamic_bitset.hpp>

#include <cstdint>
#include <limits>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Bytes = casil::Bytes;

namespace boost { using Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(Bytes_Tests)

BOOST_AUTO_TEST_CASE(Test1_composeByteVec)
{
    using Bytes::composeByteVec;

    BOOST_CHECK_EQUAL((composeByteVec(true, std::uint8_t(0x12u), std::uint16_t(0x3456u), std::uint32_t(0x789ABCDEu),
                                            std::uint64_t(0xF0E1D2C3B4A59687u))),
                      (std::vector<std::uint8_t>{0x12u,
                                                 0x34u, 0x56u,
                                                 0x78u, 0x9Au, 0xBCu, 0xDEu,
                                                 0xF0u, 0xE1u, 0xD2u, 0xC3u, 0xB4u, 0xA5u, 0x96u, 0x87u}));

    BOOST_CHECK_EQUAL((composeByteVec(false, std::uint8_t(0x12u), std::uint16_t(0x3456u), std::uint32_t(0x789ABCDEu),
                                             std::uint64_t(0xF0E1D2C3B4A59687u))),
                      (std::vector<std::uint8_t>{0x12u,
                                                 0x56u, 0x34u,
                                                 0xDEu, 0xBCu, 0x9Au, 0x78u,
                                                 0x87u, 0x96u, 0xA5u, 0xB4u, 0xC3u, 0xD2u, 0xE1u, 0xF0u}));
}

BOOST_AUTO_TEST_CASE(Test2_composeUIntNTypes)
{
    using Bytes::composeUInt16;
    using Bytes::composeUInt32;
    using Bytes::composeUInt64;

    BOOST_CHECK_EQUAL(composeUInt16(std::vector<std::uint8_t>{0x12u, 0x34u}, true), std::uint16_t(0x1234u));
    BOOST_CHECK_EQUAL(composeUInt16(std::span<const std::uint8_t, 2>({0x12u, 0x34u}), true), std::uint16_t(0x1234u));

    BOOST_CHECK_EQUAL(composeUInt16(std::vector<std::uint8_t>{0x12u, 0x34u}, false), std::uint16_t(0x3412u));
    BOOST_CHECK_EQUAL(composeUInt16(std::span<const std::uint8_t, 2>({0x12u, 0x34u}), false), std::uint16_t(0x3412u));

    BOOST_CHECK_EQUAL(composeUInt32(std::vector<std::uint8_t>{0x12u, 0x34u, 0x56u, 0x78u}, true), std::uint32_t(0x12345678u));
    BOOST_CHECK_EQUAL(composeUInt32(std::span<const std::uint8_t, 4>({0x12u, 0x34u, 0x56u, 0x78u}), true), std::uint32_t(0x12345678u));

    BOOST_CHECK_EQUAL(composeUInt32(std::vector<std::uint8_t>{0x12u, 0x34u, 0x56u, 0x78u}, false), std::uint32_t(0x78563412u));
    BOOST_CHECK_EQUAL(composeUInt32(std::span<const std::uint8_t, 4>({0x12u, 0x34u, 0x56u, 0x78u}), false), std::uint32_t(0x78563412u));

    BOOST_CHECK_EQUAL(composeUInt64(std::vector<std::uint8_t>{0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u}, true),
                      std::uint64_t(0x123456789ABCDEF0u));
    BOOST_CHECK_EQUAL(composeUInt64(std::span<const std::uint8_t, 8>({0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u}), true),
                      std::uint64_t(0x123456789ABCDEF0u));

    BOOST_CHECK_EQUAL(composeUInt64(std::vector<std::uint8_t>{0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u}, false),
                      std::uint64_t(0xF0DEBC9A78563412u));
    BOOST_CHECK_EQUAL(composeUInt64(std::span<const std::uint8_t, 8>({0x12u, 0x34u, 0x56u, 0x78u, 0x9Au, 0xBCu, 0xDEu, 0xF0u}), false),
                      std::uint64_t(0xF0DEBC9A78563412u));
}

BOOST_AUTO_TEST_CASE(Test3_composeUIntNTypesException)
{
    using Bytes::composeUInt16;
    using Bytes::composeUInt32;
    using Bytes::composeUInt64;

    int exceptionCtr = 0;

    try { std::uint16_t x = composeUInt16({0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { std::uint16_t x = composeUInt16({0x00, 0x00, 0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { std::uint32_t x = composeUInt32({0x00, 0x00, 0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { std::uint32_t x = composeUInt32({0x00, 0x00, 0x00, 0x00, 0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { std::uint64_t x = composeUInt64({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    try { std::uint64_t x = composeUInt64({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}); (void)x; }
    catch (const std::invalid_argument&) { ++exceptionCtr; }

    BOOST_CHECK_EQUAL(exceptionCtr, 6);
}

BOOST_AUTO_TEST_CASE(Test4_bitsetFromBytes)
{
    using Bytes::bitsetFromBytes;
    using boost::dynamic_bitset;

    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b1010101u}, 7), dynamic_bitset(std::string("1010101")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b1100111u}, 7), dynamic_bitset(std::string("1100111")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b1100111u}, 11), dynamic_bitset(std::string("00001100111")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b1100111u}, 3), dynamic_bitset(std::string("111")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b1100111u}, 0), dynamic_bitset(std::string("")));

    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b11010101u}, 8), dynamic_bitset(std::string("11010101")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b11001111u}, 8), dynamic_bitset(std::string("11001111")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b11001111u}, 5), dynamic_bitset(std::string("01111")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b11001111u}, 9), dynamic_bitset(std::string("011001111")));

    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b101011, 0b10001001}, 14),
                      dynamic_bitset(std::string("10101110001001")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b111100, 0b00100011}, 14),
                      dynamic_bitset(std::string("11110000100011")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b111100, 0b00100011}, 24),
                      dynamic_bitset(std::string("000000000011110000100011")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b111100, 0b00100011}, 6),
                      dynamic_bitset(std::string("100011")));

    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001}, 24),
                      dynamic_bitset(std::string("111110111011011010001001")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}, 24),
                      dynamic_bitset(std::string("000100100101101110100010")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}, 25),
                      dynamic_bitset(std::string("0000100100101101110100010")));
    BOOST_CHECK_EQUAL(bitsetFromBytes(std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}, 23),
                      dynamic_bitset(std::string("00100100101101110100010")));
}

BOOST_AUTO_TEST_CASE(Test5_bytesFromBitset)
{
    using Bytes::bytesFromBitset;
    using boost::dynamic_bitset;

    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("1010101")), 1), (std::vector<std::uint8_t>{0b1010101u}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("1100111")), 1), (std::vector<std::uint8_t>{0b1100111u}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("1100111")), 0), (std::vector<std::uint8_t>{}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("1100111")), 2), (std::vector<std::uint8_t>{0b0, 0b1100111u}));

    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11010101")), 1), (std::vector<std::uint8_t>{0b11010101u}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11001111")), 1), (std::vector<std::uint8_t>{0b11001111u}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11001111")), 0), (std::vector<std::uint8_t>{}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11001111")), 2), (std::vector<std::uint8_t>{0b0, 0b11001111u}));

    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("10101110001001")), 2),
                      (std::vector<std::uint8_t>{0b101011, 0b10001001}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11110000100011")), 2),
                      (std::vector<std::uint8_t>{0b111100, 0b00100011}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11110000100011")), 1),
                      (std::vector<std::uint8_t>{0b00100011}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("11110000100011")), 5),
                      (std::vector<std::uint8_t>{0b0, 0b0, 0b0, 0b111100, 0b00100011}));

    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("111110111011011010001001")), 3),
                      (std::vector<std::uint8_t>{0b11111011, 0b10110110, 0b10001001}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("000100100101101110100010")), 3),
                      (std::vector<std::uint8_t>{0b00010010, 0b01011011, 0b10100010}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("000100100101101110100010")), 2),
                      (std::vector<std::uint8_t>{0b01011011, 0b10100010}));
    BOOST_CHECK_EQUAL(bytesFromBitset(dynamic_bitset(std::string("000100100101101110100010")), 4),
                      (std::vector<std::uint8_t>{0b0, 0b00010010, 0b01011011, 0b10100010}));
}

BOOST_AUTO_TEST_CASE(Test6_byteVecToFromStrConversion)
{
    using Bytes::byteVecFromStr;
    using Bytes::strFromByteVec;

    BOOST_CHECK((byteVecFromStr(strFromByteVec({1, 2, 3, 4, 5, 9, 8, 7, 6})) == std::vector<std::uint8_t>{1, 2, 3, 4, 5, 9, 8, 7, 6}));
    BOOST_CHECK(strFromByteVec(byteVecFromStr("Hello World")) == "Hello World");

    BOOST_CHECK((byteVecFromStr("abcdefQ") == std::vector<std::uint8_t>{97, 98, 99, 100, 101, 102, 81}));
    BOOST_CHECK(strFromByteVec({90, 48, 67, 36, 45}) == "Z0C$-");
}

BOOST_AUTO_TEST_CASE(Test7_appendToByteVec)
{
    using Bytes::appendToByteVec;

    std::vector<std::uint8_t> v{1, 2, 3};

    appendToByteVec(v, {});

    BOOST_CHECK_EQUAL(v, (std::vector<std::uint8_t>{1, 2, 3}));

    appendToByteVec(v, {4, 5, 6, 7, 8, 9});

    BOOST_CHECK_EQUAL(v, (std::vector<std::uint8_t>{1, 2, 3, 4, 5, 6, 7, 8, 9}));
}

BOOST_AUTO_TEST_CASE(Test6_toStringFormatting)
{
    using Bytes::formatHex;
    using Bytes::formatByteVec;
    using Bytes::formatUInt16Vec;
    using Bytes::formatUInt32Vec;
    using Bytes::formatUInt64Vec;
    using Bytes::operator<<;

    BOOST_CHECK_EQUAL(formatHex(std::uint8_t{0xFu}), "0xF");
    BOOST_CHECK_EQUAL(formatHex(std::uint8_t{0xABu}), "0xAB");
    BOOST_CHECK_EQUAL(formatHex(std::uint16_t{0xABu}), "0xAB");
    BOOST_CHECK_EQUAL(formatHex(std::uint16_t{0xABCu}), "0xABC");
    BOOST_CHECK_EQUAL(formatHex(std::uint32_t{0x12345679u}), "0x12345679");
    BOOST_CHECK_EQUAL(formatHex(std::uint64_t{0x12345679ABCDEFu}), "0x12345679ABCDEF");
    BOOST_CHECK_EQUAL(formatHex(std::uint64_t{0x12345679ABCDEF01u}), "0x12345679ABCDEF01");

    BOOST_CHECK_EQUAL(formatHex(std::uint8_t{0xFu}, true), "0x0F");
    BOOST_CHECK_EQUAL(formatHex(std::uint8_t{0xABu}, true), "0xAB");
    BOOST_CHECK_EQUAL(formatHex(std::uint16_t{0xABu}, true), "0x00AB");
    BOOST_CHECK_EQUAL(formatHex(std::uint16_t{0xABCu}, true), "0x0ABC");
    BOOST_CHECK_EQUAL(formatHex(std::uint32_t{0x45679u}, true), "0x00045679");
    BOOST_CHECK_EQUAL(formatHex(std::uint64_t{0x12345679ABCDEFu}, true), "0x0012345679ABCDEF");
    BOOST_CHECK_EQUAL(formatHex(std::uint64_t{0x12345679ABCDEF01u}, true), "0x12345679ABCDEF01");

    std::ostringstream osstr;
    osstr<<"TESTBEGIN-"<<std::vector<std::uint8_t>{0, 1, 10, 20, 64, 251, 255}<<"-TESTEND";

    BOOST_CHECK_EQUAL(osstr.str(), "TESTBEGIN-{0x0, 0x1, 0xA, 0x14, 0x40, 0xFB, 0xFF}-TESTEND");

    std::ostringstream osstr2;
    osstr2<<"TESTBEGIN-"<<std::vector<std::uint16_t>{std::numeric_limits<std::uint16_t>::max(), 0, 1, 1000, 2048, 64, 251}<<"-TESTEND";

    BOOST_CHECK_EQUAL(osstr2.str(), "TESTBEGIN-{0xFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");


    std::ostringstream osstr3;
    osstr3<<"TESTBEGIN-"<<std::vector<std::uint32_t>{std::numeric_limits<std::uint32_t>::max(), 0, 1, 1000, 2048, 64, 251}<<"-TESTEND";

    BOOST_CHECK_EQUAL(osstr3.str(), "TESTBEGIN-{0xFFFFFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");

    std::ostringstream osstr4;
    osstr4<<"TESTBEGIN-"<<std::vector<std::uint64_t>{std::numeric_limits<std::uint64_t>::max(), 0, 1, 1000, 2048, 64, 251}<<"-TESTEND";

    BOOST_CHECK_EQUAL(osstr4.str(), "TESTBEGIN-{0xFFFFFFFFFFFFFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");

    std::string testStr = std::string("TESTBEGIN-").append(Bytes::formatByteVec({0, 1, 10, 20, 64, 251, 255})).append("-TESTEND");

    BOOST_CHECK_EQUAL(testStr, "TESTBEGIN-{0x0, 0x1, 0xA, 0x14, 0x40, 0xFB, 0xFF}-TESTEND");

    std::string testStr2 = std::string("TESTBEGIN-").append(Bytes::formatUInt16Vec({std::numeric_limits<std::uint16_t>::max(),
                                                                                    0, 1, 1000, 2048, 64, 251})).append("-TESTEND");

    BOOST_CHECK_EQUAL(testStr2, "TESTBEGIN-{0xFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");


    std::string testStr3 = std::string("TESTBEGIN-").append(Bytes::formatUInt32Vec({std::numeric_limits<std::uint32_t>::max(),
                                                                                    0, 1, 1000, 2048, 64, 251})).append("-TESTEND");

    BOOST_CHECK_EQUAL(testStr3, "TESTBEGIN-{0xFFFFFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");

    std::string testStr4 = std::string("TESTBEGIN-").append(Bytes::formatUInt64Vec({std::numeric_limits<std::uint64_t>::max(),
                                                                                    0, 1, 1000, 2048, 64, 251})).append("-TESTEND");

    BOOST_CHECK_EQUAL(testStr4, "TESTBEGIN-{0xFFFFFFFFFFFFFFFF, 0x0, 0x1, 0x3E8, 0x800, 0x40, 0xFB}-TESTEND");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
