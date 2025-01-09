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

#include <pycasil/pycasil.h>

#include <casil/env.h>

#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace Env = casil::Env;

void bindEnv(py::module& pM)
{
    pM.def("getEnv", static_cast<const std::map<std::string, std::set<std::string>, std::less<>>& (*)(void)>(&Env::getEnv),
           "Get a map of all Casil environment variables.", py::return_value_policy::reference);
    pM.def("getEnv", static_cast<const std::set<std::string>& (*)(std::string_view)>(&Env::getEnv),
           "Get a specific Casil environment variable.", py::arg("varName"), py::return_value_policy::reference);
}
