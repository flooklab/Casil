/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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

#include <casil/TL/Direct/dummyinterface.h>

using casil::TL::DummyInterface;

void bindTL_DummyInterface(py::module& pM)
{
    py::class_<DummyInterface, casil::TL::DirectInterface>(pM, "DummyInterface",
                                                           "Dummy implementation of DirectInterface without actual functionality.")
            .def(py::init<std::string, casil::LayerConfig>(), "Constructor.", py::arg("name"), py::arg("config"));
}
