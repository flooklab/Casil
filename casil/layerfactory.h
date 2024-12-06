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

#ifndef CASIL_LAYERFACTORY_H
#define CASIL_LAYERFACTORY_H

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace casil
{

class LayerConfig;

namespace TL { class Interface; }
namespace HL { class Driver; }
namespace RL { class Register; }

/*!
 * \brief Factory for LayerBase classes of the three component layers TL, HL and RL.
 *
 * \todo Detailed doc
 */
class LayerFactory
{
public:
    typedef std::function<std::unique_ptr<TL::Interface>(std::string, LayerConfig)> TLGeneratorFunction;
                                                                                    ///< Function signature required for interface generators.
    typedef std::function<std::unique_ptr<HL::Driver>(std::string, TL::Interface&, LayerConfig)> HLGeneratorFunction;
                                                                                    ///< Function signature required for driver generators.
    typedef std::function<std::unique_ptr<RL::Register>(std::string, HL::Driver&, LayerConfig)> RLGeneratorFunction;
                                                                                    ///< Function signature required for register generators.

public:
    LayerFactory() = delete;                                                                ///< Deleted constructor.
    //
    static std::unique_ptr<TL::Interface> createInterface(const std::string& pType, std::string pName, LayerConfig pConfig);
                                                                                            ///< Construct a registered interface type.
    static std::unique_ptr<HL::Driver> createDriver(const std::string& pType, std::string pName, TL::Interface& pInterface, LayerConfig pConfig);
                                                                                            ///< Construct a registered driver type.
    static std::unique_ptr<RL::Register> createRegister(const std::string& pType, std::string pName, HL::Driver& pDriver, LayerConfig pConfig);
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
