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

/*!
 * \file layerfactorymacros.h
 *
 * \brief Macro function definitions to simplify factory registration of layer components.
 *
 * See \ref casil::LayerFactory "LayerFactory" for detailed information about the registration of layer components.
 * These macros here can/should be used in favor of the raw \ref casil::LayerFactory "LayerFactory" functionality
 * in order to simplify and unify the registration of component classes.
*/

/*!
 * \brief Register an interface component class's type name.
 *
 * Append this to a \ref casil::Layers::TL::Interface "TL::Interface" class definition in order to define a
 * type name via the \c typeName member, which is needed by \ref CASIL_REGISTER_INTERFACE_CPP for registering
 * the component type to the \ref casil::LayerFactory "LayerFactory". The presence of this member is also used by
 * the \ref TemplateDeviceSpecialization "TemplateDevice" configuration helpers (see \ref casil::TmplDev "TmplDev"
 * and \ref casil::Concepts::HasRegisteredTypeName "Concepts::HasRegisteredTypeName")
 * to check at compile time that the type is actually registered.
 *
 * \param TYPE_NAME The type name to register to the factory for the class.
 */
#define CASIL_REGISTER_INTERFACE_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

/*!
 * \brief Register a driver component class's type name.
 *
 * Append this to a \ref casil::Layers::HL::Driver "HL::Driver" class definition in order to define a type name via the \c typeName member,
 * which is needed by \ref CASIL_REGISTER_DRIVER_CPP for registering the component type to the \ref casil::LayerFactory "LayerFactory".
 * The presence of this member is also used by the \ref TemplateDeviceSpecialization "TemplateDevice" configuration helpers
 * (see \ref casil::TmplDev "TmplDev" and \ref casil::Concepts::HasRegisteredTypeName "Concepts::HasRegisteredTypeName")
 * to check at compile time that the type is actually registered.
 *
 * \param TYPE_NAME The type name to register to the factory for the class.
 */
#define CASIL_REGISTER_DRIVER_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

/*!
 * \brief Register a register component class's type name.
 *
 * Append this to a \ref casil::Layers::RL::Register "RL::Register" class definition in order to define a type name via the \c typeName member,
 * which is needed by \ref CASIL_REGISTER_REGISTER_CPP for registering the component type to the \ref casil::LayerFactory "LayerFactory".
 * The presence of this member is also used by the \ref TemplateDeviceSpecialization "TemplateDevice" configuration helpers
 * (see \ref casil::TmplDev "TmplDev" and \ref casil::Concepts::HasRegisteredTypeName "Concepts::HasRegisteredTypeName")
 * to check at compile time that the type is actually registered.
 *
 * \param TYPE_NAME The type name to register to the factory for the class.
 */
#define CASIL_REGISTER_REGISTER_H(TYPE_NAME) \
public:\
    static constexpr char typeName[] = TYPE_NAME;\
private:

//

/*!
 * \brief Register an interface component to the \ref casil::LayerFactory "LayerFactory".
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::TL::Interface "TL::Interface" class
 * in order to register the component type to the \ref casil::LayerFactory "LayerFactory", which is needed to
 * use the component with \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice".
 * You need to also place \ref CASIL_REGISTER_INTERFACE_H within the class's definition to make this work,
 * because that macro defines the actual type name that is used to refer to the component type.
 *
 * \note You can only register one class per translation unit (this macro utilizes the unnamed namespace of the translation unit).
 *
 * \param TYPE_CLASS The class type to register to the factory.
 */
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

/*!
 * \brief Register a driver component to the \ref casil::LayerFactory "LayerFactory".
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::HL::Driver "HL::Driver" class
 * in order to register the component type to the \ref casil::LayerFactory "LayerFactory", which is needed to
 * use the component with \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice".
 * You need to also place \ref CASIL_REGISTER_DRIVER_H within the class's definition to make this work,
 * because that macro defines the actual type name that is used to refer to the component type.
 *
 * \note You can only register one class per translation unit (this macro utilizes the unnamed namespace of the translation unit).
 *
 * \note The defined generator function for \ref casil::LayerFactory "LayerFactory" catches all potential \c std::bad_cast
 *       exceptions from the class constructor and in that case throws \c std::runtime_error instead. This also
 *       applies to the attempted conversion of the interface instance that is passed to the constructor/factory,
 *       i.e. if the interface type has a wrong base type (direct vs. muxed), the generator throws \c std::runtime_error.
 *
 * \param TYPE_CLASS The class type to register to the factory.
 */
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

/*!
 * \brief Register a register component to the \ref casil::LayerFactory "LayerFactory".
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::RL::Register "HL::Register" class
 * in order to register the component type to the \ref casil::LayerFactory "LayerFactory", which is needed to
 * use the component with \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice".
 * You need to also place \ref CASIL_REGISTER_REGISTER_H within the class's definition to make this work,
 * because that macro defines the actual type name that is used to refer to the component type.
 *
 * \note You can only register one class per translation unit (this macro utilizes the unnamed namespace of the translation unit).
 *
 * \param TYPE_CLASS The class type to register to the factory.
 */
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

//Append the content a macro to an expression
#define CASIL_CONCATENATE_HELPER(X, Y) X##Y
#define CASIL_CONCATENATE_WITH_MACRO(X, Y) CASIL_CONCATENATE_HELPER(X, Y)

/*!
 * \brief Register an alias type name for an interface component.
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::TL::Interface "TL::Interface" class \e after
 * \ref CASIL_REGISTER_INTERFACE_CPP in order to allow using this component type with the \ref casil::LayerFactory "LayerFactory"
 * and \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice" under an alternative type name
 * (i.e. different from the one defined by \ref CASIL_REGISTER_INTERFACE_H).
 * Use this macro multiple times to define multiple different aliases.
 *
 * \note You can only register aliases for the \e one component of the current translation unit (this macro utilizes and refers
 * to the the unnamed namespace of the translation unit that was previously opened by \ref CASIL_REGISTER_INTERFACE_CPP).
 *
 * \param ALIAS_NAME An alternative type name to additionally register to the factory for the class.
 */
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

/*!
 * \brief Register an alias type name for a driver component.
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::HL::Driver "HL::Driver" class \e after
 * \ref CASIL_REGISTER_DRIVER_CPP in order to allow using this component type with the \ref casil::LayerFactory "LayerFactory"
 * and \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice" under an alternative type name
 * (i.e. different from the one defined by \ref CASIL_REGISTER_DRIVER_H).
 * Use this macro multiple times to define multiple different aliases.
 *
 * \note You can only register aliases for the \e one component of the current translation unit (this macro utilizes and refers
 * to the the unnamed namespace of the translation unit that was previously opened by \ref CASIL_REGISTER_DRIVER_CPP).
 *
 * \param ALIAS_NAME An alternative type name to additionally register to the factory for the class.
 */
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

/*!
 * \brief Register an alias type name for a register component.
 *
 * Put this into the translation unit (the source file) of a \ref casil::Layers::RL::Register "RL::Register" class \e after
 * \ref CASIL_REGISTER_REGISTER_CPP in order to allow using this component type with the \ref casil::LayerFactory "LayerFactory"
 * and \ref casil::Device "Device" / \ref TemplateDeviceSpecialization "TemplateDevice" under an alternative type name
 * (i.e. different from the one defined by \ref CASIL_REGISTER_REGISTER_H).
 * Use this macro multiple times to define multiple different aliases.
 *
 * \note You can only register aliases for the \e one component of the current translation unit (this macro utilizes and refers
 * to the the unnamed namespace of the translation unit that was previously opened by \ref CASIL_REGISTER_REGISTER_CPP).
 *
 * \param ALIAS_NAME An alternative type name to additionally register to the factory for the class.
 */
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
