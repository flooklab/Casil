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
    typedef std::function<std::unique_ptr<HL::Driver>(std::string, TL::Interface&, LayerConfig)> HLGeneratorFunction;
    typedef std::function<std::unique_ptr<RL::Register>(std::string, HL::Driver&, LayerConfig)> RLGeneratorFunction;

public:
    LayerFactory() = delete;
    //
    static std::unique_ptr<TL::Interface> createInterface(const std::string& pType, std::string pName, LayerConfig pConfig);
    static std::unique_ptr<HL::Driver> createDriver(const std::string& pType, std::string pName, TL::Interface& pInterface,
                                                    LayerConfig pConfig);
    static std::unique_ptr<RL::Register> createRegister(const std::string& pType, std::string pName, HL::Driver& pDriver,
                                                        LayerConfig pConfig);
    //
    static void registerInterfaceType(std::string pType, TLGeneratorFunction pGenerator);
    static void registerDriverType(std::string pType, HLGeneratorFunction pGenerator);
    static void registerRegisterType(std::string pType, RLGeneratorFunction pGenerator);
    //
    static void registerInterfaceAlias(const std::string& pType, std::string pAlias);
    static void registerDriverAlias(const std::string& pType, std::string pAlias);
    static void registerRegisterAlias(const std::string& pType, std::string pAlias);

private:
    static std::map<std::string, TLGeneratorFunction>& tlGenerators();
    static std::map<std::string, HLGeneratorFunction>& hlGenerators();
    static std::map<std::string, RLGeneratorFunction>& rlGenerators();
};

} // namespace casil

#endif // CASIL_LAYERFACTORY_H
