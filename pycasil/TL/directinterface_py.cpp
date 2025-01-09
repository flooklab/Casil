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

#include <casil/TL/directinterface.h>

using casil::TL::DirectInterface;

void bindTL_DirectInterface(py::module& pM)
{
    py::class_<DirectInterface, casil::TL::Interface>(pM, "DirectInterface", "Base class to derive from for interface components "
                                                                             "that directly connect to an independent hardware device.")
            .def("read", &DirectInterface::read, "Read from the interface.", py::arg("size") = -1)
            .def("write", &DirectInterface::write, "Write to the interface.", py::arg("data"))
            .def("query", &DirectInterface::query, "Write a query to the interface and read the response.",
                 py::arg("data"), py::arg("size") = -1);
}
