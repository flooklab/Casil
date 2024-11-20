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

#include <casil/TL/Muxed/sitcp.h>

using casil::TL::SiTCP;

void bindTL_SiTCP(py::module& pM)
{
    py::class_<SiTCP, casil::TL::MuxedInterface>(pM, "SiTcp", "")
            .def(py::init<std::string, casil::LayerConfig>(), "", py::arg("name"), py::arg("config"))
            .def("resetFifo", &SiTCP::resetFifo, "")
            .def("resetFifoMod32", &SiTCP::resetFifoMod32, "")
            .def("getFifoSize", &SiTCP::getFifoSize, "")
            .def("getFifoData", &SiTCP::getFifoData, "", py::arg("size") = -1)
            .def_readonly_static("baseAddrDataLimit", &SiTCP::baseAddrDataLimit, "")
            .def_readonly_static("baseAddrFIFOLimit", &SiTCP::baseAddrFIFOLimit, "");
}
