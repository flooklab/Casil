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

#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/layerconfig.h>

#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <string>
#include <vector>

using casil::LayerConfig;

namespace boost { using casil::Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(LayerConfig_Tests)

BOOST_AUTO_TEST_CASE(Test1_fromYAML)
{
    std::string yamlStr = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: none, nested: [{one: 1}, {two: 2}]}}";

    BOOST_CHECK(LayerConfig(casil::Auxil::propertyTreeFromYAML(yamlStr)) == LayerConfig::fromYAML(yamlStr));
}

BOOST_AUTO_TEST_CASE(Test2_selfContained)
{
    std::string yamlStr = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: none, nested: [{one: 1}, {two: 2}]}}";

    boost::property_tree::ptree pTree = casil::Auxil::propertyTreeFromYAML(yamlStr);

    LayerConfig conf1(pTree);
    LayerConfig conf2(pTree);

    BOOST_CHECK(conf1.contains(conf2) == true);
}

BOOST_AUTO_TEST_CASE(Test3_containsStuff)
{
    std::string yamlStr = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: none, nested: [{one: 1}, {two: 2}]}}";
    std::string toBeContainedStr = "{init: {port: \"foo\", read_termination: bar, baudrate: 123, nested: hello}}";
    std::string notToBeContainedStr = "{init: {porto: \"foo\"}}";

    LayerConfig conf = LayerConfig::fromYAML(yamlStr);
    LayerConfig containedConf = LayerConfig::fromYAML(toBeContainedStr);
    LayerConfig notContainedConf = LayerConfig::fromYAML(notToBeContainedStr);

    BOOST_CHECK(conf.contains(containedConf) == true);
    BOOST_CHECK(conf.contains(notContainedConf) == false);
}

BOOST_AUTO_TEST_CASE(Test4_containsTypes)
{
    std::string yamlStr = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: 19200, limit: -1, addr: 0x10, flag: True,"
                                  "nested: [{one: 1}, {two: 2a}]}, s1: [1,2,3], s2: [-1, -2, -3], s3: [1024, 2048, 486, 45]}";

    std::string toBeContainedStr = "{init: {port: string, read_termination: string, baudrate: int, limit: int, addr: uint, flag: bool,"
                                   "nested: [{one: double}]}, s1: byteSeq, s3: uintSeq}";

    std::string notToBeContainedStr1 = "{init: {port: int, read_termination: string, baudrate: int, limit: int, addr: uint, flag: bool,"
                                       "nested: [{one: double}]}, s1: byteSeq, s3: uintSeq}";
    std::string notToBeContainedStr2 = "{init: {port: string, read_termination: string, baudrate: int, limit: uint, addr: uint, flag: bool,"
                                       "nested: [{one: double}]}, s1: byteSeq, s3: uintSeq}";
    std::string notToBeContainedStr3 = "{init: {port: string, read_termination: string, baudrate: int, limit: int, addr: uint, flag: bool,"
                                       "nested: [{one: double}]}, s2: uintSeq}";
    std::string notToBeContainedStr4 = "{init: {port: string, read_termination: string, baudrate: int, limit: int, addr: uint, flag: bool,"
                                       "nested: [{one: double}]}, s3: byteSeq}";

    LayerConfig conf = LayerConfig::fromYAML(yamlStr);
    LayerConfig containedConf = LayerConfig::fromYAML(toBeContainedStr);
    LayerConfig notContainedConf1 = LayerConfig::fromYAML(notToBeContainedStr1);
    LayerConfig notContainedConf2 = LayerConfig::fromYAML(notToBeContainedStr2);
    LayerConfig notContainedConf3 = LayerConfig::fromYAML(notToBeContainedStr3);
    LayerConfig notContainedConf4 = LayerConfig::fromYAML(notToBeContainedStr4);

    BOOST_CHECK(conf.contains(containedConf, true) == true);
    BOOST_CHECK(conf.contains(notContainedConf1, true) == false);
    BOOST_CHECK(conf.contains(notContainedConf2, true) == false);
    BOOST_CHECK(conf.contains(notContainedConf3, true) == false);
    BOOST_CHECK(conf.contains(notContainedConf4, true) == false);
}

BOOST_AUTO_TEST_CASE(Test5_getValues)
{
    std::string yamlStr = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: 19200, limit: -1, addr: 0x10,"
                                  "nested: [{one: 1.3}, {two: 2a, three: True}]},"
                                  "s1: [1,2,3], s2: [-1, -2, -3], s3: [1024, 2048, 486, 45], s4: [], s5: {z: 74, a: 73}}";

    LayerConfig conf = LayerConfig::fromYAML(yamlStr);

    BOOST_CHECK_EQUAL(conf.getStr("init.port", "port1"), "/dev/ttyUSB1");
    BOOST_CHECK_EQUAL(conf.getStr("init.read_termination", "\r\n"), "\n\r");
    BOOST_CHECK_EQUAL(conf.getInt("init.baudrate", 123), 19200);
    BOOST_CHECK_EQUAL(conf.getInt("init.limit", 55), -1);
    BOOST_CHECK_EQUAL(conf.getInt("init.limits", -4), -4);
    BOOST_CHECK_EQUAL(conf.getUInt("init.addr", 0), 16u);
    BOOST_CHECK_EQUAL(conf.getUInt("init.address", 0xAAu), 0xAAu);
    BOOST_CHECK_EQUAL(conf.getDbl("init.nested.#0.one", 0), 1.3);
    BOOST_CHECK_EQUAL(conf.getInt("init.nested.#1.two", 1013), 1013);
    BOOST_CHECK_EQUAL(conf.getBool("init.nested.#1.three", false), true);
    BOOST_CHECK_EQUAL(conf.getBool("init.nested.#1.four", false), false);
    BOOST_CHECK_EQUAL(conf.getStr("init.foo", "bar"), "bar");

    BOOST_CHECK_EQUAL(conf.getByteSeq("s1", {99}), (std::vector<std::uint8_t>{1, 2, 3}));
    BOOST_CHECK_EQUAL(conf.getByteSeq("s2", {99}), (std::vector<std::uint8_t>{99}));
    BOOST_CHECK_EQUAL(conf.getByteSeq("s3", {99}), (std::vector<std::uint8_t>{99}));
    BOOST_CHECK_EQUAL(conf.getByteSeq("s4", {99}), (std::vector<std::uint8_t>{}));
    BOOST_CHECK_EQUAL(conf.getByteSeq("s5", {99}), (std::vector<std::uint8_t>{74, 73}));

    BOOST_CHECK_EQUAL(conf.getUIntSeq("s1", {99}), (std::vector<std::uint64_t>{1, 2, 3}));
    BOOST_CHECK_EQUAL(conf.getUIntSeq("s2", {99}), (std::vector<std::uint64_t>{99}));
    BOOST_CHECK_EQUAL(conf.getUIntSeq("s3", {99}), (std::vector<std::uint64_t>{1024, 2048, 486, 45}));
    BOOST_CHECK_EQUAL(conf.getUIntSeq("s4", {99}), (std::vector<std::uint64_t>{}));
    BOOST_CHECK_EQUAL(conf.getUIntSeq("s5", {99}), (std::vector<std::uint64_t>{74, 73}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
