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

#include <casil/auxil.h>
#include <casil/bytes.h>

#include <boost/property_tree/ptree.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Auxil = casil::Auxil;

namespace boost { using casil::Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(Auxil_Tests)

BOOST_AUTO_TEST_CASE(Test1_ptreeFromYAML)
{
    const std::filesystem::path testDirPath = std::filesystem::path(dataPath) / "core" / "test_auxil";

    std::ifstream yamlFile;

    yamlFile.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    yamlFile.open(testDirPath / "bdaq53.yaml");

    std::string yamlStr;

    try
    {
        for (std::string line; std::getline(yamlFile, line, '\n');)
            yamlStr.append(line).append("\n");
    }
    catch (const std::ios_base::failure&)
    {
        if (!yamlFile.eof())
            throw;
    }

    yamlFile.close();

    using boost::property_tree::ptree;

    ptree tree = Auxil::propertyTreeFromYAML(yamlStr);

    std::ostringstream ostrm;

    std::function<void(const ptree&, int)> readTree = [&readTree, &ostrm](const ptree& pTree, const int pLevel) -> void
    {
        std::string indent;
        for (int i = 0; i < pLevel; ++i)
            indent += "\t";

        ostrm<<" DATA: "<<pTree.data()<<"\n";

        for (const auto& it : pTree)
        {
            ostrm<<indent<<it.first;
            readTree(it.second, pLevel+1);
        }
    };

    readTree(tree, 0);

    const std::string testStr = ostrm.str();

    std::ifstream refFile;

    refFile.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    refFile.open(testDirPath / "bdaq53_ref.txt");

    std::string refStr;

    try
    {
        for (std::string line; std::getline(refFile, line, '\n');)
            refStr.append(line).append("\n");
    }
    catch (const std::ios_base::failure&)
    {
        if (!yamlFile.eof())
            throw;
    }

    refFile.close();

    BOOST_CHECK_EQUAL(testStr, refStr);

    const ptree treeEmptyYamlStr = Auxil::propertyTreeFromYAML("");

    BOOST_CHECK(treeEmptyYamlStr == ptree());
}

BOOST_AUTO_TEST_CASE(Test2_ptreeFromYAMLParserException)
{
    const std::string invalidYAMLStr = "{init: ";

    bool exceptionThrown = false;

    try
    {
        boost::property_tree::ptree tree = Auxil::propertyTreeFromYAML(invalidYAMLStr);
        (void)tree;
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_CASE(Test3_ptreeToYAML)
{
    using boost::property_tree::ptree;

    const std::string yamlStrOrig1 = "";
    const std::string yamlStrOrig2 = "{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: 19200, limit: -1, addr: 0x10,"
                                             "nested: [{one: 1.3}, {two: 2a, three: True}]},"
                                      "s1: [1,2,3], s2: [-1, -2, -3], s3: [1024, 2048, 486, 45], s4: [], s5: {z: 74, a: 73}}";

    const ptree tree1 = Auxil::propertyTreeFromYAML(yamlStrOrig1);
    const ptree tree2 = Auxil::propertyTreeFromYAML(yamlStrOrig2);

    const std::string yamlStrBackConv1 = Auxil::propertyTreeToYAML(tree1);
    const std::string yamlStrBackConv2 = Auxil::propertyTreeToYAML(tree2);

    const ptree treeBackConv1 = Auxil::propertyTreeFromYAML(yamlStrBackConv1);
    const ptree treeBackConv2 = Auxil::propertyTreeFromYAML(yamlStrBackConv2);

    BOOST_CHECK(tree1 == treeBackConv1);
    BOOST_CHECK_EQUAL(yamlStrOrig1, yamlStrBackConv1);

    BOOST_CHECK(tree2 == treeBackConv2);    //Do not compare YAML strings here because of lack of formatting in original string

    const std::string yamlStrDefaultTree = Auxil::propertyTreeToYAML(ptree());

    BOOST_CHECK_EQUAL(yamlStrDefaultTree, "");
}

BOOST_AUTO_TEST_CASE(Test4_uintSeqFromYAML)
{
    using Auxil::uintSeqFromYAML;

    BOOST_CHECK_EQUAL((uintSeqFromYAML("[1,2,3,4,5]")), (std::vector<std::uint64_t>{1, 2, 3, 4, 5}));
    BOOST_CHECK_EQUAL((uintSeqFromYAML("[1,2,3,4,]")), (std::vector<std::uint64_t>{1, 2, 3, 4}));
    BOOST_CHECK_EQUAL((uintSeqFromYAML("[1024, 0x2, 0x3, 0xFF]")), (std::vector<std::uint64_t>{1024, 2, 3, 255}));
    BOOST_CHECK_EQUAL((uintSeqFromYAML("[]")), (std::vector<std::uint64_t>{}));
    BOOST_CHECK_EQUAL((uintSeqFromYAML("{foo: 12, bar: 34}")), (std::vector<std::uint64_t>{12, 34}));

    bool exceptionThrown = false;
    try { (void)(uintSeqFromYAML("[-1]")); }
    catch (const std::runtime_error&) { exceptionThrown = true; }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_CASE(Test5_chrono)
{
    using Auxil::getChronoMilliSecs;
    using Auxil::getChronoMicroSecs;

    using namespace std::chrono_literals;

    BOOST_CHECK_EQUAL(getChronoMilliSecs(0), 0ms);
    BOOST_CHECK_EQUAL(getChronoMilliSecs(10e-3), 10ms);
    BOOST_CHECK_EQUAL(getChronoMilliSecs(5.0044), 5004ms);
    BOOST_CHECK_EQUAL(getChronoMilliSecs(5.0045), 5005ms);
    BOOST_CHECK_EQUAL(getChronoMilliSecs(-1.5e-3), -2ms);

    BOOST_CHECK_EQUAL(getChronoMicroSecs(0), 0us);
    BOOST_CHECK_EQUAL(getChronoMicroSecs(10e-3), 10000us);
    BOOST_CHECK_EQUAL(getChronoMicroSecs(5.0000044), 5000004us);
    BOOST_CHECK_EQUAL(getChronoMicroSecs(5.0000045), 5000005us);
    BOOST_CHECK_EQUAL(getChronoMicroSecs(-1.5e-6), -2us);
}

BOOST_AUTO_TEST_CASE(Test6_asioRunner)
{
    Auxil::AsyncIORunner<2> ioRunner;
    (void)ioRunner;

    BOOST_REQUIRE(casil::ASIO::ioContextThreadsRunning());

    bool secondRunnerFailed = false;

    try
    {
        Auxil::AsyncIORunner<2> ioRunner2;
        (void)ioRunner2;
    }
    catch (const std::runtime_error&)
    {
        secondRunnerFailed = true;
    }

    BOOST_CHECK(secondRunnerFailed);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
