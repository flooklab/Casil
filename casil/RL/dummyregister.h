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

#ifndef CASIL_RL_DUMMYREGISTER_H
#define CASIL_RL_DUMMYREGISTER_H

#include <casil/RL/register.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/HL/driver.h>

#include <string>

namespace casil
{

namespace RL
{

class DummyRegister final : public Register
{
public:
    DummyRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig);
    ~DummyRegister() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_REGISTER_H("DummyRegister")
};

} // namespace RL

} // namespace casil

#endif // CASIL_RL_DUMMYREGISTER_H
