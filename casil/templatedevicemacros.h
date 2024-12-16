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

#ifndef CASIL_TEMPLATEDEVICEMACROS_H
#define CASIL_TEMPLATEDEVICEMACROS_H

#include <casil/templatedevice.h>

//

/*!
 * \file templatedevicemacros.h
 *
 * \brief Macro function definitions to simplify configuration of layer components for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * These macros here can be used to avoid having to \e manually define a \ref TemplateDeviceSpecialization "TemplateDevice" configuration
 * struct (see \ref casil::TmplDev::InterfaceConf "TmplDev::InterfaceConf" / \ref casil::TmplDev::DriverConf "TmplDev::DriverConf" /
 * \ref casil::TmplDev::RegisterConf "TmplDev::RegisterConf") for every layer component.
*/

/*!
 * \brief Define an interface configuration struct for use with \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * Defines an interface configuration wrapper derived from \ref casil::TmplDev::InterfaceConf "TmplDev::InterfaceConf"
 * such that it meets the concept requirement \ref casil::TmplDev::ImplementsInterfaceConf "TmplDev::ImplementsInterfaceConf"
 * and can hence be used to configure an interface component for \ref TemplateDeviceSpecialization "TemplateDevice".
 * To use it in that way \p STRUCT_NAME must be used as template argument of \ref casil::TmplDev::InterfacesConf "TmplDev::InterfacesConf".
 *
 * \param INTF_CLASS The registered \ref casil::Layers::TL::Interface "TL::Interface" class type to use as component type.
 * \param STRUCT_NAME The desired struct name of the configuration wrapper to be declared.
 * \param INTF_NAME Component instance name (as string literal).
 * \param CONF Component configuration YAML code (as string literal).
 */
#define CASIL_DEFINE_INTERFACE(INTF_CLASS, STRUCT_NAME, INTF_NAME, CONF) \
struct STRUCT_NAME : public casil::TmplDev::InterfaceConf<INTF_CLASS>\
{\
    static constexpr char name[] = INTF_NAME;\
    static constexpr char conf[] = CONF;\
};

/*!
 * \brief Define a driver configuration struct for use with \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * Defines a driver configuration wrapper derived from \ref casil::TmplDev::DriverConf "TmplDev::DriverConf"
 * such that it meets the concept requirement \ref casil::TmplDev::ImplementsDriverConf "TmplDev::ImplementsDriverConf"
 * and can hence be used to configure a driver component for \ref TemplateDeviceSpecialization "TemplateDevice".
 * To use it in that way \p STRUCT_NAME must be used as template argument of \ref casil::TmplDev::DriversConf "TmplDev::DriversConf".
 *
 * \param DRV_CLASS The registered \ref casil::Layers::HL::Driver "HL::Driver" class type to use as component type.
 * \param STRUCT_NAME The desired struct name of the configuration wrapper to be declared.
 * \param DRV_NAME Component instance name (as string literal).
 * \param INTERFACE Configured instance name of the interface component to be used (as string literal).
 * \param CONF Component configuration YAML code (as string literal).
 */
#define CASIL_DEFINE_DRIVER(DRV_CLASS, STRUCT_NAME, DRV_NAME, INTERFACE, CONF) \
struct STRUCT_NAME : public casil::TmplDev::DriverConf<DRV_CLASS>\
{\
    static constexpr char name[] = DRV_NAME;\
    static constexpr char interface[] = INTERFACE;\
    static constexpr char conf[] = CONF;\
};

/*!
 * \brief Define a register configuration struct for use with \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * Defines a register configuration wrapper derived from \ref casil::TmplDev::RegisterConf "TmplDev::RegisterConf"
 * such that it meets the concept requirement \ref casil::TmplDev::ImplementsRegisterConf "TmplDev::ImplementsRegisterConf"
 * and can hence be used to configure a register component for \ref TemplateDeviceSpecialization "TemplateDevice".
 * To use it in that way \p STRUCT_NAME must be used as template argument of \ref casil::TmplDev::RegistersConf "TmplDev::RegistersConf".
 *
 * \param REG_CLASS The registered \ref casil::Layers::RL::Register "RL::Register" class type to use as component type.
 * \param STRUCT_NAME The desired struct name of the configuration wrapper to be declared.
 * \param REG_NAME Component instance name (as string literal).
 * \param DRIVER Configured instance name of the driver component to be used (as string literal).
 * \param CONF Component configuration YAML code (as string literal).
 */
#define CASIL_DEFINE_REGISTER(REG_CLASS, STRUCT_NAME, REG_NAME, DRIVER, CONF) \
struct STRUCT_NAME : public casil::TmplDev::RegisterConf<REG_CLASS>\
{\
    static constexpr char name[] = REG_NAME;\
    static constexpr char driver[] = DRIVER;\
    static constexpr char conf[] = CONF;\
};

//

#endif // CASIL_TEMPLATEDEVICEMACROS_H
