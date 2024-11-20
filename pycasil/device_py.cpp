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

#include <casil/device.h>

using casil::Device;

void bind_Device(py::module& pM)
{
    py::class_<Device>(pM, "Device", "")
            .def(py::init<const std::string&>(), "", py::arg("config"))
            .def("__getitem__", &Device::operator[], "", py::arg("name"), py::return_value_policy::reference, py::is_operator())
            .def("interface", &Device::interface, "", py::arg("name"), py::return_value_policy::reference)
            .def("driver", &Device::driver, "", py::arg("name"), py::return_value_policy::reference)
            .def("reg", &Device::reg, "", py::arg("name"), py::return_value_policy::reference)
            .def("init", &Device::init, "", py::arg("force") = false)
            .def("close", &Device::close, "", py::arg("force") = false);
}
