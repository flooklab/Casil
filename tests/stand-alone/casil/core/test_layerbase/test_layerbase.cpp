/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#include "layertestclass.h"
#include "rtconftestclass.h"

#include <casil/layerbase.h>
#include <casil/layerconfig.h>

#include <stdexcept>

using casil::LayerConfig;

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(LayerBase_Tests)

BOOST_AUTO_TEST_CASE(Test1_validConfig)
{
    bool exceptionThrown = false;

    try
    {
        LayerTestClass layerTestClass(LayerConfig::fromYAML("{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: 19200}}"),
                                      LayerConfig::fromYAML("{init: {port: string, read_termination: string, baudrate: int}}"));
        (void)layerTestClass;
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == false);
}

BOOST_AUTO_TEST_CASE(Test2_invalidConfig1)
{
    bool exceptionThrown = false;

    try
    {
        LayerTestClass layerTestClass(LayerConfig::fromYAML("{init: {port: /dev/ttyUSB1, baudrate: 19200}}"),
                                      LayerConfig::fromYAML("{init: {port: string, read_termination: string, baudrate: int}}"));
        (void)layerTestClass;
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_CASE(Test3_invalidConfig2)
{
    bool exceptionThrown = false;

    try
    {
        LayerTestClass layerTestClass(LayerConfig::fromYAML("{init: {port: /dev/ttyUSB1, read_termination: \"\\n\\r\", baudrate: none}}"),
                                      LayerConfig::fromYAML("{init: {port: string, read_termination: string, baudrate: int}}"));
        (void)layerTestClass;
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_CASE(Test4_emptyConfig)
{
    bool exceptionThrown = false;

    try
    {
        LayerTestClass layerTestClass = LayerTestClass(LayerConfig(), LayerConfig());
        (void)layerTestClass;
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == false);
}

BOOST_AUTO_TEST_CASE(Test5_initClose)
{
    using casil::LayerBase;

    std::unique_ptr<LayerBase> layerTestClass1 = std::make_unique<LayerTestClass>(LayerConfig(), LayerConfig(), false, false);

    BOOST_CHECK(layerTestClass1->init() == true);
    BOOST_CHECK(layerTestClass1->close() == true);

    BOOST_CHECK(layerTestClass1->init() == true);
    BOOST_CHECK(layerTestClass1->init() == true);
    BOOST_CHECK(layerTestClass1->init(true) == true);

    BOOST_CHECK(layerTestClass1->close() == true);
    BOOST_CHECK(layerTestClass1->close() == true);
    BOOST_CHECK(layerTestClass1->close(true) == true);

    //Let close() fail

    std::unique_ptr<LayerBase> layerTestClass2 = std::make_unique<LayerTestClass>(LayerConfig(), LayerConfig(), false, true);

    BOOST_CHECK(layerTestClass2->init() == true);
    BOOST_CHECK(layerTestClass2->close() == false);
    BOOST_CHECK(layerTestClass2->close() == false);
    BOOST_CHECK(layerTestClass2->close(true) == false);
    BOOST_CHECK(layerTestClass2->init() == true);
    BOOST_CHECK(layerTestClass2->close() == false);

    //Let init() fail

    std::unique_ptr<LayerBase> layerTestClass3 = std::make_unique<LayerTestClass>(LayerConfig(), LayerConfig(), true, false);

    BOOST_CHECK(layerTestClass3->init() == false);
    BOOST_CHECK(layerTestClass3->init() == false);
    BOOST_CHECK(layerTestClass3->init(true) == false);
    BOOST_CHECK(layerTestClass3->close() == true);

    //Let init() and close() fail

    std::unique_ptr<LayerBase> layerTestClass4 = std::make_unique<LayerTestClass>(LayerConfig(), LayerConfig(), true, true);

    BOOST_CHECK(layerTestClass4->init() == false);
    BOOST_CHECK(layerTestClass4->init() == false);
    BOOST_CHECK(layerTestClass4->init(true) == false);

    BOOST_CHECK(layerTestClass4->close() == true);      //Must succeed because was never initialized
    BOOST_CHECK(layerTestClass4->close() == true);
    BOOST_CHECK(layerTestClass4->close(true) == false); //Must fail because forcibly closing
    BOOST_CHECK(layerTestClass4->close() == true);
}

BOOST_AUTO_TEST_CASE(Test6_runtimeConfiguration)
{
    LayerTestClass layerTestClass = LayerTestClass(LayerConfig(), LayerConfig());
    RTConfTestClass rtConfTestClass = RTConfTestClass(LayerConfig(), LayerConfig());

    BOOST_REQUIRE(layerTestClass.init());
    BOOST_REQUIRE(rtConfTestClass.init());

    BOOST_CHECK_EQUAL(layerTestClass.dumpRuntimeConfiguration(), "");
    BOOST_CHECK(layerTestClass.loadRuntimeConfiguration("") == true);
    BOOST_CHECK(layerTestClass.loadRuntimeConfiguration("{init: ") == false);   //Invalid YAML code

    BOOST_CHECK_EQUAL(rtConfTestClass.dumpRuntimeConfiguration(), "some_number: 2");
    BOOST_CHECK(rtConfTestClass.loadRuntimeConfiguration("{some_number: 107}") == true);
    BOOST_CHECK_EQUAL(rtConfTestClass.dumpRuntimeConfiguration(), "some_number: 107");

    BOOST_REQUIRE(rtConfTestClass.loadRuntimeConfiguration("{some_number: 59}") == true);   //Dumping should deliberately fail now
    bool exceptionThrown = false;

    try
    {
        (void)rtConfTestClass.dumpRuntimeConfiguration();
    }
    catch (const std::runtime_error&)
    {
        exceptionThrown = true;
    }

    BOOST_CHECK(exceptionThrown == true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
