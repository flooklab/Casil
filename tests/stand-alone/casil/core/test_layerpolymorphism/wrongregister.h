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

#ifndef CASILTESTS_WRONGREGISTER_H
#define CASILTESTS_WRONGREGISTER_H

#include <casil/RL/register.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/HL/driver.h>

#include <string>

namespace casil
{

class WrongRegister final : public RL::Register
{
public:
    WrongRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig);
    ~WrongRegister() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_REGISTER_H("WrongRegister")
};

} // namespace casil

#endif // CASILTESTS_WRONGREGISTER_H
