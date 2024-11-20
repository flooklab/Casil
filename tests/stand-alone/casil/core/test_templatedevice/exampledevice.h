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

#ifndef CASILTESTS_EXAMPLEDEVICE_H
#define CASILTESTS_EXAMPLEDEVICE_H

#include "testdriver.h"

#include <casil/templatedevice.h>
#include <casil/HL/Direct/dummydriver.h>
#include <casil/RL/dummyregister.h>
#include <casil/TL/Direct/dummyinterface.h>

struct TLDummyInterface1 : public casil::TmplDevInterface<casil::TL::DummyInterface>
{
    static constexpr char name[] = "DummyInterface1";
    static constexpr char conf[] = "";
};

struct HLDummyDriver1 : public casil::TmplDevDriver<casil::HL::DummyDriver>
{
    static constexpr char name[] = "DummyDriver1";
    static constexpr char interface[] = "DummyInterface1";
    static constexpr char conf[] = "";
};

struct HLDriver2 : public casil::TmplDevDriver<casil::HL::TestDriver>
{
    static constexpr char name[] = "testdrv123";
    static constexpr char interface[] = "DummyInterface1";
    static constexpr char conf[] = "";
};

struct RLDummyRegister1 : public casil::TmplDevRegister<casil::RL::DummyRegister>
{
    static constexpr char name[] = "DummyRegister1";
    static constexpr char driver[] = "DummyDriver1";
    static constexpr char conf[] = "";
};

typedef casil::TemplateDevice<
                casil::TmplDevInterfaces<TLDummyInterface1>,
                casil::TmplDevDrivers<HLDummyDriver1,
                                      HLDriver2>,
                casil::TmplDevRegisters<RLDummyRegister1>
            > ExampleDevice;

#endif // CASILTESTS_EXAMPLEDEVICE_H
