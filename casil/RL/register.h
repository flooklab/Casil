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

#ifndef CASIL_RL_REGISTER_H
#define CASIL_RL_REGISTER_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>
#include <casil/HL/driver.h>

#include <string>

namespace casil
{

namespace RL
{

class Register : public LayerBase
{
public:
    Register(std::string pType, std::string pName, HL::Driver& pDriver, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
    ~Register() override = default;

private:
    bool initImpl() override = 0;
    bool closeImpl() override = 0;

protected:
    HL::Driver& driver;
};

} // namespace RL

} // namespace casil

#endif // CASIL_RL_REGISTER_H
