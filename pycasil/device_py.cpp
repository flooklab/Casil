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

#include <casil/device.h>

using casil::Device;

void bind_Device(py::module& pM)
{
    py::class_<Device>(pM, "Device",
                       "Configurable container class for interdependent layer components to interact with an arbitrary DAQ setup.")
            .def(py::init<const std::string&>(), "Constructor.", py::arg("config"))
            .def("__getitem__", &Device::operator[], "Access one of the components from any layer.",
                 py::arg("name"), py::return_value_policy::reference, py::is_operator())
            .def("interface", &Device::interface, "Access one of the interface components from the transfer layer.",
                 py::arg("name"), py::return_value_policy::reference)
            .def("driver", &Device::driver, "Access one of the driver components from the hardware layer.",
                 py::arg("name"), py::return_value_policy::reference)
            .def("reg", &Device::reg, "Access one of the register components from the register layer.",
                 py::arg("name"), py::return_value_policy::reference)
            .def("init", &Device::init, "Initialize by initializing all components of all layers.", py::arg("force") = false)
            .def("close", &Device::close, "Close by closing all components of all layers.", py::arg("force") = false)
            .def("loadRuntimeConfiguration", &Device::loadRuntimeConfiguration,
                 "Load additional runtime configuration data/values for the components.", py::arg("conf"))
            .def("dumpRuntimeConfiguration", &Device::dumpRuntimeConfiguration,
                 "Save current runtime configuration data/values of the components.");
}
