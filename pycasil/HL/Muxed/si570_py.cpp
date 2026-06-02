/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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

#include <casil/HL/Muxed/si570.h>

using casil::HL::Si570;

void bindHL_Si570(py::module& pM)
{
    py::class_<Si570, casil::HL::MetaDriver>(pM, "Si570",
                                             "Meta driver for the Si570 clock generator IC that is interfaced via an i2c firmware module.")
            .def(py::init<std::string, casil::HL::MuxedDriver&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("backendDriver"), py::arg("config"))
            .def("reset", &Si570::reset, "Reset the device.")
            .def("changeFrequency", &Si570::changeFrequency, "Change reference frequency of Si570 device.", py::arg("freqMHz"));
}
