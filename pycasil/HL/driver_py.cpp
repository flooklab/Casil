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

#include <casil/HL/driver.h>

using casil::HL::Driver;

void bindHL_Driver(py::module& pM)
{
    py::class_<Driver, casil::LayerBase>(pM, "Driver", "")
            .def("reset", &Driver::reset, "")
            .def("getData", &Driver::getData, "", py::arg("size") = -1, py::arg("addrOffs") = 0u)
            .def("setData", &Driver::setData, "", py::arg("data"), py::arg("addrOffs") = 0u)
            .def("exec", &Driver::exec, "")
            .def("isDone", &Driver::isDone, "");
}
