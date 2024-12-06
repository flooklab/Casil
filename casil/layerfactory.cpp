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

#include <casil/layerfactory.h>

#include <casil/layerconfig.h>
#include <casil/HL/driver.h>
#include <casil/RL/register.h>
#include <casil/TL/interface.h>

#include <stdexcept>
#include <utility>

using casil::LayerFactory;

using casil::TL::Interface;
using casil::HL::Driver;
using casil::RL::Register;

//Public

/*!
 * \brief Construct a registered interface type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pName
 * \param pConfig
 * \return
 */
std::unique_ptr<Interface> LayerFactory::createInterface(const std::string& pType, std::string pName, LayerConfig pConfig)
{
    const auto it = tlGenerators().find(pType);

    if (it == tlGenerators().end())
        return nullptr;

    const TLGeneratorFunction& genFunc = it->second;

    try
    {
        return genFunc(std::move(pName), std::move(pConfig));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Error while constructing interface: ") + exc.what());
    }

    return nullptr;
}

/*!
 * \brief Construct a registered driver type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pName
 * \param pInterface
 * \param pConfig
 * \return
 */
std::unique_ptr<Driver> LayerFactory::createDriver(const std::string& pType, std::string pName, Interface& pInterface, LayerConfig pConfig)
{
    const auto it = hlGenerators().find(pType);

    if (it == hlGenerators().end())
        return nullptr;

    const HLGeneratorFunction& genFunc = it->second;

    try
    {
        return genFunc(std::move(pName), pInterface, std::move(pConfig));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Error while constructing driver: ") + exc.what());
    }

    return nullptr;
}

/*!
 * \brief Construct a registered register type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pName
 * \param pDriver
 * \param pConfig
 * \return
 */
std::unique_ptr<Register> LayerFactory::createRegister(const std::string& pType, std::string pName, Driver& pDriver, LayerConfig pConfig)
{
    const auto it = rlGenerators().find(pType);

    if (it == rlGenerators().end())
        return nullptr;

    const RLGeneratorFunction& genFunc = it->second;

    try
    {
        return genFunc(std::move(pName), pDriver, std::move(pConfig));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Error while constructing register: ") + exc.what());
    }

    return nullptr;
}

//

/*!
 * \brief Register a generator for an interface type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pGenerator
 */
void LayerFactory::registerInterfaceType(std::string pType, TLGeneratorFunction pGenerator)
{
    tlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

/*!
 * \brief Register a generator for an driver type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pGenerator
 */
void LayerFactory::registerDriverType(std::string pType, HLGeneratorFunction pGenerator)
{
    hlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

/*!
 * \brief Register a generator for an register type.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pGenerator
 */
void LayerFactory::registerRegisterType(std::string pType, RLGeneratorFunction pGenerator)
{
    rlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

//

/*!
 * \brief Register an interface type name alias.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pAlias
 */
void LayerFactory::registerInterfaceAlias(const std::string& pType, std::string pAlias)
{
    const auto it = tlGenerators().find(pType);

    if (it == tlGenerators().end())
        return;

    TLGeneratorFunction boundGenerator = std::bind(it->second, std::placeholders::_1, std::placeholders::_2);

    tlGenerators().insert({std::move(pAlias), std::move(boundGenerator)});
}

/*!
 * \brief Register a driver type name alias.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pAlias
 */
void LayerFactory::registerDriverAlias(const std::string& pType, std::string pAlias)
{
    const auto it = hlGenerators().find(pType);

    if (it == hlGenerators().end())
        return;

    HLGeneratorFunction boundGenerator = std::bind(it->second, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    hlGenerators().insert({std::move(pAlias), std::move(boundGenerator)});
}

/*!
 * \brief Register a register type name alias.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pAlias
 */
void LayerFactory::registerRegisterAlias(const std::string& pType, std::string pAlias)
{
    const auto it = rlGenerators().find(pType);

    if (it == rlGenerators().end())
        return;

    RLGeneratorFunction boundGenerator = std::bind(it->second, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    rlGenerators().insert({std::move(pAlias), std::move(boundGenerator)});
}

//Private

/*!
 * \brief Access the map of interface generators with interface types as keys.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::map<std::string, LayerFactory::TLGeneratorFunction>& LayerFactory::tlGenerators()
{
    static std::map<std::string, TLGeneratorFunction> typeGtors = {};
    return typeGtors;
}

/*!
 * \brief Access the map of driver generators with driver types as keys.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::map<std::string, LayerFactory::HLGeneratorFunction>& LayerFactory::hlGenerators()
{
    static std::map<std::string, HLGeneratorFunction> typeGtors = {};
    return typeGtors;
}

/*!
 * \brief Access the map of register generators with register types as keys.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::map<std::string, LayerFactory::RLGeneratorFunction>& LayerFactory::rlGenerators()
{
    static std::map<std::string, RLGeneratorFunction> typeGtors = {};
    return typeGtors;
}
