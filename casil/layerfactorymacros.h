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

#ifndef CASIL_LAYERFACTORYMACROS_H
#define CASIL_LAYERFACTORYMACROS_H

#include <casil/layerconfig.h>
#include <casil/layerfactory.h>
#include <casil/HL/driver.h>
#include <casil/RL/register.h>
#include <casil/TL/interface.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

//

#define CASIL_REGISTER_INTERFACE_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

#define CASIL_REGISTER_DRIVER_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

#define CASIL_REGISTER_REGISTER_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

//

#define CASIL_REGISTER_INTERFACE_CPP(TYPE_CLASS) \
namespace\
{\
namespace RegistryImpl\
{\
    using casil::LayerConfig;\
    using casil::LayerFactory;\
    using casil::TL::Interface;\
    using ThisInterfaceType = TYPE_CLASS;\
    class RegistryHelper\
    {\
    public:\
        RegistryHelper()\
        {\
            LayerFactory::registerInterfaceType(\
                ThisInterfaceType::typeName,\
                [](std::string pName, LayerConfig pConfig) -> std::unique_ptr<Interface>\
                {\
                    return std::make_unique<ThisInterfaceType>(std::move(pName), std::move(pConfig));\
                }\
            );\
        }\
    };\
    const RegistryHelper registryHelper;\
}\
}

#define CASIL_REGISTER_DRIVER_CPP(TYPE_CLASS) \
namespace\
{\
namespace RegistryImpl\
{\
    using casil::LayerConfig;\
    using casil::LayerFactory;\
    using casil::HL::Driver;\
    using casil::TL::Interface;\
    using ThisDriverType = TYPE_CLASS;\
    class RegistryHelper\
    {\
    public:\
        RegistryHelper()\
        {\
            LayerFactory::registerDriverType(\
                ThisDriverType::typeName,\
                [](std::string pName, Interface& pInterface, LayerConfig pConfig) -> std::unique_ptr<Driver>\
                {\
                    try\
                    {\
                        return std::make_unique<ThisDriverType>(std::move(pName),\
                                                                dynamic_cast<ThisDriverType::InterfaceBaseType&>(pInterface),\
                                                                std::move(pConfig));\
                    }\
                    catch (const std::bad_cast&)\
                    {\
                        throw std::runtime_error("Incompatible interface type \"" + pInterface.getType() + "\" for use with \"" +\
                                                 ThisDriverType::typeName + "\".");\
                    }\
                }\
            );\
        }\
    };\
    const RegistryHelper registryHelper;\
}\
}

#define CASIL_REGISTER_REGISTER_CPP(TYPE_CLASS) \
namespace\
{\
namespace RegistryImpl\
{\
    using casil::LayerConfig;\
    using casil::LayerFactory;\
    using casil::HL::Driver;\
    using casil::RL::Register;\
    using ThisRegisterType = TYPE_CLASS;\
    class RegistryHelper\
    {\
    public:\
        RegistryHelper()\
        {\
            LayerFactory::registerRegisterType(\
                ThisRegisterType::typeName,\
                [](std::string pName, Driver& pDriver, LayerConfig pConfig) -> std::unique_ptr<Register>\
                {\
                    return std::make_unique<ThisRegisterType>(std::move(pName), pDriver, std::move(pConfig));\
                }\
            );\
        }\
    };\
    const RegistryHelper registryHelper;\
}\
}

//

#define CASIL_CONCATENATE_HELPER(X, Y) X##Y
#define CASIL_CONCATENATE_WITH_MACRO(X, Y) CASIL_CONCATENATE_HELPER(X, Y)

#define CASIL_REGISTER_INTERFACE_ALIAS(ALIAS_NAME) \
namespace\
{\
namespace RegistryImpl\
{\
    class CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)\
    {\
    public:\
        CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)()\
        {\
            LayerFactory::registerInterfaceAlias(ThisInterfaceType::typeName, ALIAS_NAME);\
        }\
    };\
    CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__) CASIL_CONCATENATE_WITH_MACRO(aliasHelper, __LINE__);\
}\
}

#define CASIL_REGISTER_DRIVER_ALIAS(ALIAS_NAME) \
namespace\
{\
namespace RegistryImpl\
{\
    class CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)\
    {\
    public:\
        CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)()\
        {\
            LayerFactory::registerDriverAlias(ThisDriverType::typeName, ALIAS_NAME);\
        }\
    };\
    CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__) CASIL_CONCATENATE_WITH_MACRO(aliasHelper, __LINE__);\
}\
}

#define CASIL_REGISTER_REGISTER_ALIAS(ALIAS_NAME) \
namespace\
{\
namespace RegistryImpl\
{\
    class CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)\
    {\
    public:\
        CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__)()\
        {\
            LayerFactory::registerRegisterAlias(ThisRegisterType::typeName, ALIAS_NAME);\
        }\
    };\
    CASIL_CONCATENATE_WITH_MACRO(AliasHelper, __LINE__) CASIL_CONCATENATE_WITH_MACRO(aliasHelper, __LINE__);\
}\
}

//

#endif // CASIL_LAYERFACTORYMACROS_H
