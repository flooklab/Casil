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

#include <casil/TL/muxedinterface.h>

using casil::TL::MuxedInterface;

void bindTL_MuxedInterface(py::module& pM)
{
    py::class_<MuxedInterface, casil::TL::Interface>(pM, "MuxedInterface", "Base class to derive from for interface components that connect "
                                                                           "to an FPGA endpoint running the basil bus and firmware modules.")
            .def("read", &MuxedInterface::read, "Read from the interface.", py::arg("addr"), py::arg("size") = -1)
            .def("write", &MuxedInterface::write, "Write to the interface.", py::arg("addr"), py::arg("data"))
            .def("query", &MuxedInterface::query, "Write a query to the interface and read the response.",
                 py::arg("writeAddr"), py::arg("readAddr"), py::arg("data"), py::arg("size") = -1);
}
