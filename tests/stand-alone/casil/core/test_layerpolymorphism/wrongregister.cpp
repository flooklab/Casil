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

#include "wrongregister.h"

#include <utility>

using casil::WrongRegister;

CASIL_REGISTER_REGISTER_CPP(WrongRegister)

//

WrongRegister::WrongRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig) :
    Register(typeName, std::move(pName), pDriver, std::move(pConfig), LayerConfig())
{
}

//Private

bool WrongRegister::initImpl()
{
    return true;
}

bool WrongRegister::closeImpl()
{
    return true;
}
