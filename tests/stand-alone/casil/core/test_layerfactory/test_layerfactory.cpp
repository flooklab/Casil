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

#include <casil/layerbase.h>
#include <casil/layerconfig.h>
#include <casil/layerfactory.h>
#include <casil/HL/directdriver.h>
#include <casil/HL/driver.h>
#include <casil/HL/Direct/dummydriver.h>
#include <casil/RL/register.h>
#include <casil/RL/dummyregister.h>
#include <casil/TL/interface.h>
#include <casil/TL/directinterface.h>
#include <casil/TL/Direct/dummyinterface.h>

#include <memory>
#include <string>
#include <utility>

using casil::LayerBase;
using casil::LayerConfig;
using casil::LayerFactory;
using casil::TL::Interface;
using casil::TL::DirectInterface;
using casil::TL::DummyInterface;
using casil::HL::Driver;
using casil::HL::DirectDriver;
using casil::HL::DummyDriver;
using casil::RL::Register;
using casil::RL::DummyRegister;

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(LayerFactory_Tests)

BOOST_AUTO_TEST_CASE(Test1_dummiesMacroRegistered)
{
    std::unique_ptr<Interface> testIntf = LayerFactory::createInterface("DummyInterface", "dummyIntf1", LayerConfig());

    BOOST_REQUIRE(testIntf != nullptr);

    std::unique_ptr<Driver> testDrv = LayerFactory::createDriver("DummyDriver", "dummyDrv1", *testIntf, LayerConfig());

    BOOST_REQUIRE(testDrv != nullptr);

    std::unique_ptr<Register> testReg = LayerFactory::createRegister("DummyRegister", "dummyReg1", *testDrv, LayerConfig());

    BOOST_REQUIRE(testReg != nullptr);

    BOOST_CHECK(testIntf->getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(testIntf->getType(), "DummyInterface");
    BOOST_CHECK_EQUAL(testIntf->getName(), "dummyIntf1");

    BOOST_CHECK(testDrv->getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(testDrv->getType(), "DummyDriver");
    BOOST_CHECK_EQUAL(testDrv->getName(), "dummyDrv1");

    BOOST_CHECK(testReg->getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(testReg->getType(), "DummyRegister");
    BOOST_CHECK_EQUAL(testReg->getName(), "dummyReg1");
}

BOOST_AUTO_TEST_CASE(Test2_interface)
{
    const std::string typeName = "foobar-testInterface";
    const std::string typeNameAlias = "foobar-alias-testInterface";

    std::unique_ptr<Interface> testIntf1 = LayerFactory::createInterface(typeName, "foobar-testIntf-1", LayerConfig());

    BOOST_CHECK(testIntf1 == nullptr);

    auto createInterface = [](std::string pName, LayerConfig pConfig) -> std::unique_ptr<Interface>
    {
        return std::make_unique<DummyInterface>(std::move(pName), std::move(pConfig));
    };

    LayerFactory::registerInterfaceType(typeName, createInterface);
    LayerFactory::registerInterfaceAlias(typeName, typeNameAlias);

    std::unique_ptr<Interface> testIntf2 = LayerFactory::createInterface(typeName, "foobar-testIntf-2", LayerConfig());

    BOOST_CHECK(testIntf2 != nullptr);
    BOOST_CHECK(testIntf2->getLayer() == LayerBase::Layer::TransferLayer);
    BOOST_CHECK_EQUAL(testIntf2->getType(), "DummyInterface");
    BOOST_CHECK_EQUAL(testIntf2->getName(), "foobar-testIntf-2");

    std::unique_ptr<Interface> testIntf3 = LayerFactory::createInterface(typeNameAlias, "foobar-testIntf-3", LayerConfig());

    BOOST_CHECK_EQUAL(testIntf3->getType(), testIntf2->getType());
}

BOOST_AUTO_TEST_CASE(Test3_driver)
{
    std::unique_ptr<Interface> tInterface = LayerFactory::createInterface("DummyInterface", "tInterface", LayerConfig());

    BOOST_REQUIRE(tInterface != nullptr);

    const std::string typeName = "foobar-testDriver";
    const std::string typeNameAlias = "foobar-alias-testDriver";

    std::unique_ptr<Driver> testDrv1 = LayerFactory::createDriver(typeName, "foobar-testDrv-1", *tInterface, LayerConfig());

    BOOST_CHECK(testDrv1 == nullptr);

    auto createDriver = [](std::string pName, Interface& pInterface, LayerConfig pConfig) -> std::unique_ptr<Driver>
    {
        return std::make_unique<DummyDriver>(std::move(pName), dynamic_cast<DirectInterface&>(pInterface), std::move(pConfig));
    };

    LayerFactory::registerDriverType(typeName, createDriver);
    LayerFactory::registerDriverAlias(typeName, typeNameAlias);

    std::unique_ptr<Driver> testDrv2 = LayerFactory::createDriver(typeName, "foobar-testDrv-2", *tInterface, LayerConfig());

    BOOST_CHECK(testDrv2 != nullptr);
    BOOST_CHECK(testDrv2->getLayer() == LayerBase::Layer::HardwareLayer);
    BOOST_CHECK_EQUAL(testDrv2->getType(), "DummyDriver");
    BOOST_CHECK_EQUAL(testDrv2->getName(), "foobar-testDrv-2");

    std::unique_ptr<Driver> testDrv3 = LayerFactory::createDriver(typeNameAlias, "foobar-testDrv-3", *tInterface, LayerConfig());

    BOOST_CHECK_EQUAL(testDrv3->getType(), testDrv2->getType());
}

BOOST_AUTO_TEST_CASE(Test4_register)
{
    std::unique_ptr<Interface> tInterface = LayerFactory::createInterface("DummyInterface", "tInterface", LayerConfig());

    BOOST_REQUIRE(tInterface != nullptr);

    std::unique_ptr<Driver> tDriver = LayerFactory::createDriver("DummyDriver", "tDriver", *tInterface, LayerConfig());

    BOOST_REQUIRE(tDriver != nullptr);

    const std::string typeName = "foobar-testRegister";
    const std::string typeNameAlias = "foobar-alias-testRegister";

    std::unique_ptr<Register> testReg1 = LayerFactory::createRegister(typeName, "foobar-testReg-1", *tDriver, LayerConfig());

    BOOST_CHECK(testReg1 == nullptr);

    auto createRegister = [](std::string pName, Driver& pDriver, LayerConfig pConfig) -> std::unique_ptr<Register>
    {
        return std::make_unique<DummyRegister>(std::move(pName), pDriver, std::move(pConfig));
    };

    LayerFactory::registerRegisterType(typeName, createRegister);
    LayerFactory::registerRegisterAlias(typeName, typeNameAlias);

    std::unique_ptr<Register> testReg2 = LayerFactory::createRegister(typeName, "foobar-testReg-2", *tDriver, LayerConfig());

    BOOST_CHECK(testReg2 != nullptr);
    BOOST_CHECK(testReg2->getLayer() == LayerBase::Layer::RegisterLayer);
    BOOST_CHECK_EQUAL(testReg2->getType(), "DummyRegister");
    BOOST_CHECK_EQUAL(testReg2->getName(), "foobar-testReg-2");

    std::unique_ptr<Register> testReg3 = LayerFactory::createRegister(typeNameAlias, "foobar-testReg-3", *tDriver, LayerConfig());

    BOOST_CHECK_EQUAL(testReg3->getType(), testReg2->getType());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
