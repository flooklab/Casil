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

#define CASIL_DEFINE_INTERFACE(INTF_CLASS, STRUCT_NAME, INTF_NAME, CONF) \
struct STRUCT_NAME : public casil::TmplDevInterface<INTF_CLASS>\
{\
    static constexpr char name[] = INTF_NAME;\
    static constexpr char conf[] = CONF;\
};

#define CASIL_DEFINE_DRIVER(DRV_CLASS, STRUCT_NAME, DRV_NAME, INTERFACE, CONF) \
struct STRUCT_NAME : public casil::TmplDevDriver<DRV_CLASS>\
{\
    static constexpr char name[] = DRV_NAME;\
    static constexpr char interface[] = INTERFACE;\
    static constexpr char conf[] = CONF;\
};

#define CASIL_DEFINE_REGISTER(REG_CLASS, STRUCT_NAME, REG_NAME, DRIVER, CONF) \
struct STRUCT_NAME : public casil::TmplDevRegister<REG_CLASS>\
{\
    static constexpr char name[] = REG_NAME;\
    static constexpr char driver[] = DRIVER;\
    static constexpr char conf[] = CONF;\
};

//

#endif // CASIL_TEMPLATEDEVICEMACROS_H
