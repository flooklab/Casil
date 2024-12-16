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
 * Calls the generator function for the registered type name \p pType (see registerInterfaceType(), registerInterfaceAlias())
 * with forwarded \p pName and \p pConfig arguments and returns a pointer to the generated TL::Interface.
 *
 * Returns \c nullptr if \p pType is not a registered interface type name.
 *
 * \param pType Registered type name (or alias) of the requested interface type.
 * \param pName Instance name for the new interface component.
 * \param pConfig Configuration for the new interface component.
 * \return Pointer to the created TL::Interface.
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
 * Calls the generator function for the registered type name \p pType (see registerDriverType(), registerDriverAlias())
 * with forwarded \p pName, \p pInterface and \p pConfig arguments and returns a pointer to the generated HL::Driver.
 *
 * Returns \c nullptr if \p pType is not a registered driver type name.
 *
 * \param pType Registered type name (or alias) of the requested driver type.
 * \param pName Instance name for the new driver component.
 * \param pInterface The interface instance for accessing the TL.
 * \param pConfig Configuration for the new driver component.
 * \return Pointer to the created HL::Driver.
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
 * Calls the generator function for the registered type name \p pType (see registerRegisterType(), registerRegisterAlias())
 * with forwarded \p pName, \p pDriver and \p pConfig arguments and returns a pointer to the generated RL::Register.
 *
 * Returns \c nullptr if \p pType is not a registered register type name.
 *
 * \param pType Registered type name (or alias) of the requested register type.
 * \param pName Instance name for the new register component.
 * \param pDriver The driver instance for accessing the HL.
 * \param pConfig Configuration for the new register component.
 * \return Pointer to the created RL::Register.
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
 * Registers an interface type \p T with a "type name" \p pType, which will enable createInterface() to generate \p T based on \p pType.
 * \p pGenerator must construct an instance of the desired \ref TL::Interface Interface type and return it as a pointer to TL::Interface.
 * See also TLGeneratorFunction.
 *
 * \param pType Type name to be used as type identifier for createInterface().
 * \param pGenerator Generator function to construct an instance of the type.
 */
void LayerFactory::registerInterfaceType(std::string pType, TLGeneratorFunction pGenerator)
{
    tlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

/*!
 * \brief Register a generator for an driver type.
 *
 * Registers a driver type \p T with a "type name" \p pType, which will enable createDriver() to generate \p T based on \p pType.
 * \p pGenerator must construct an instance of the desired \ref HL::Driver Driver type and return it as a pointer to HL::Driver.
 * See also HLGeneratorFunction.
 *
 * \param pType Type name to be used as type identifier for createDriver().
 * \param pGenerator Generator function to construct an instance of the type.
 */
void LayerFactory::registerDriverType(std::string pType, HLGeneratorFunction pGenerator)
{
    hlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

/*!
 * \brief Register a generator for an register type.
 *
 * Registers a register type \p T with a "type name" \p pType, which will enable createRegister() to generate \p T based on \p pType.
 * \p pGenerator must construct an instance of the desired \ref RL::Register Register type and return it as a pointer to RL::Register.
 * See also RLGeneratorFunction.
 *
 * \param pType Type name to be used as type identifier for createRegister().
 * \param pGenerator Generator function to construct an instance of the type.
 */
void LayerFactory::registerRegisterType(std::string pType, RLGeneratorFunction pGenerator)
{
    rlGenerators().insert({std::move(pType), std::move(pGenerator)});
}

//

/*!
 * \brief Register an interface type name alias.
 *
 * Enables an interface type identified by type name \p pType to also be
 * constructed by createInterface() using the alias type name \p pAlias.
 *
 * \param pType Originally registered type name.
 * \param pAlias New alias for \p pType.
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
 * Enables a driver type identified by type name \p pType to also be
 * constructed by createDriver() using the alias type name \p pAlias.
 *
 * \param pType Originally registered type name.
 * \param pAlias New alias for \p pType.
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
 * Enables a register type identified by type name \p pType to also be
 * constructed by createRegister() using the alias type name \p pAlias.
 *
 * \param pType Originally registered type name.
 * \param pAlias New alias for \p pType.
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
 * Creates a static map of generator functions and always returns a reference to that one.
 * Keys shall be the registered type names (see registerInterfaceType()) and aliases (see registerInterfaceAlias()).
 *
 * \return The generator map.
 */
std::map<std::string, LayerFactory::TLGeneratorFunction>& LayerFactory::tlGenerators()
{
    static std::map<std::string, TLGeneratorFunction> typeGtors = {};
    return typeGtors;
}

/*!
 * \brief Access the map of driver generators with driver types as keys.
 *
 * Creates a static map of generator functions and always returns a reference to that one.
 * Keys shall be the registered type names (see registerDriverType()) and aliases (see registerDriverAlias()).
 *
 * \return The generator map.
 */
std::map<std::string, LayerFactory::HLGeneratorFunction>& LayerFactory::hlGenerators()
{
    static std::map<std::string, HLGeneratorFunction> typeGtors = {};
    return typeGtors;
}

/*!
 * \brief Access the map of register generators with register types as keys.
 *
 * Creates a static map of generator functions and always returns a reference to that one.
 * Keys shall be the registered type names (see registerRegisterType()) and aliases (see registerRegisterAlias()).
 *
 * \return The generator map.
 */
std::map<std::string, LayerFactory::RLGeneratorFunction>& LayerFactory::rlGenerators()
{
    static std::map<std::string, RLGeneratorFunction> typeGtors = {};
    return typeGtors;
}
