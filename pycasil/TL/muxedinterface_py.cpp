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

#include <pycasil/pycasil.h>

#include <casil/TL/muxedinterface.h>

using casil::TL::MuxedInterface;

void bindTL_MuxedInterface(py::module& pM)
{
    py::class_<MuxedInterface, casil::TL::Interface>(pM, "MuxedInterface", "")
            .def("read", &MuxedInterface::read, "", py::arg("addr"), py::arg("size") = -1)
            .def("write", &MuxedInterface::write, "", py::arg("addr"), py::arg("data"))
            .def("query", &MuxedInterface::query, "", py::arg("writeAddr"), py::arg("readAddr"), py::arg("data"), py::arg("size") = -1);
}
