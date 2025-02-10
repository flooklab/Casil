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

#ifndef CASIL_LAYERFACTORY_H
#define CASIL_LAYERFACTORY_H

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace casil
{

class LayerConfig;

namespace Layers { namespace TL { class Interface; } }
namespace Layers { namespace HL { class Driver; } }
namespace Layers { namespace RL { class Register; } }

/*!
 * \brief Factory for LayerBase classes of the three component layers \ref Layers::TL "TL", \ref Layers::HL "HL" and \ref Layers::RL "RL".
 *
 * Use createInterface() / createDriver() / createRegister() to construct layer components by their registered type names.
 *
 * Component classes can be registered via registerInterfaceType() / registerDriverType() / registerRegisterType().
 * You can add aliases for registered type names via registerInterfaceAlias() / registerDriverAlias() / registerRegisterAlias().
 *
 * The actually recommended way to register any component classes is by using the macros defined in \ref layerfactorymacros.h.
 *
 * \note The components of this library do register themselves using the mentioned macros.
 */
class LayerFactory
{
private:
    using Interface = Layers::TL::Interface;    ///< \copybrief Layers::TL::Interface
    using Driver = Layers::HL::Driver;          ///< \copybrief Layers::HL::Driver
    using Register = Layers::RL::Register;      ///< \copybrief Layers::RL::Register

public:
    typedef std::function<std::unique_ptr<Interface>(std::string, LayerConfig)> TLGeneratorFunction;
                                                                                    ///< Function signature required for interface generators.
    typedef std::function<std::unique_ptr<Driver>(std::string, Interface&, LayerConfig)> HLGeneratorFunction;
                                                                                    ///< Function signature required for driver generators.
    typedef std::function<std::unique_ptr<Register>(std::string, Driver&, LayerConfig)> RLGeneratorFunction;
                                                                                    ///< Function signature required for register generators.

public:
    LayerFactory() = delete;                                                                ///< Deleted constructor.
    //
    static std::unique_ptr<Interface> createInterface(const std::string& pType, std::string pName, LayerConfig pConfig);
                                                                                            ///< Construct a registered interface type.
    static std::unique_ptr<Driver> createDriver(const std::string& pType, std::string pName, Interface& pInterface, LayerConfig pConfig);
                                                                                            ///< Construct a registered driver type.
    static std::unique_ptr<Register> createRegister(const std::string& pType, std::string pName, Driver& pDriver, LayerConfig pConfig);
                                                                                            ///< Construct a registered register type.
    //
    static void registerInterfaceType(std::string pType, TLGeneratorFunction pGenerator);   ///< Register a generator for an interface type.
    static void registerDriverType(std::string pType, HLGeneratorFunction pGenerator);      ///< Register a generator for an driver type.
    static void registerRegisterType(std::string pType, RLGeneratorFunction pGenerator);    ///< Register a generator for an register type.
    //
    static void registerInterfaceAlias(const std::string& pType, std::string pAlias);       ///< Register an interface type name alias.
    static void registerDriverAlias(const std::string& pType, std::string pAlias);          ///< Register a driver type name alias.
    static void registerRegisterAlias(const std::string& pType, std::string pAlias);        ///< Register a register type name alias.

private:
    static std::map<std::string, TLGeneratorFunction>& tlGenerators();  ///< Access the map of interface generators with interface types as keys.
    static std::map<std::string, HLGeneratorFunction>& hlGenerators();  ///< Access the map of driver generators with driver types as keys.
    static std::map<std::string, RLGeneratorFunction>& rlGenerators();  ///< Access the map of register generators with register types as keys.
};

} // namespace casil

#endif // CASIL_LAYERFACTORY_H
