/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
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

#include <casil/RL/standardregister.h>

using casil::RL::StandardRegister;

void bindRL_StandardRegister(py::module& pM)
{
    py::class_<StandardRegister, casil::RL::Register>(pM, "StandardRegister", "... TODO ...")
            .def(py::init<std::string, casil::HL::Driver&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("driver"), py::arg("config"));
    //TODO
}
