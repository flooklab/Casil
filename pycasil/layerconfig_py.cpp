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

#include <casil/layerconfig.h>

using casil::LayerConfig;

void bind_LayerConfig(py::module& pM)
{
    py::class_<LayerConfig>(pM, "LayerConfig", "")
            .def(py::init<>(), "")
            .def(py::init<const LayerConfig&>(), "", py::arg("other"))
            .def("__eq__", (&LayerConfig::operator==), "", py::arg("other"), py::is_operator())
            .def("contains", &LayerConfig::contains, "", py::arg("other"), py::arg("checkTypes") = false)
            .def("getBool", &LayerConfig::getBool, "", py::arg("key"), py::arg("default") = false)
            .def("getInt", &LayerConfig::getInt, "", py::arg("key"), py::arg("default") = 0)
            .def("getUInt", &LayerConfig::getUInt, "", py::arg("key"), py::arg("default") = 0u)
            .def("getDbl", &LayerConfig::getDbl, "", py::arg("key"), py::arg("default") = 0.0)
            .def("getStr", &LayerConfig::getStr, "", py::arg("key"), py::arg("default") = "")
            .def("getByteSeq", &LayerConfig::getByteSeq, "", py::arg("key"), py::arg("default") = std::vector<std::uint8_t>{})
            .def("getUIntSeq", &LayerConfig::getUIntSeq, "", py::arg("key"), py::arg("default") = std::vector<std::uint64_t>{})
            .def_static("fromYAML", &LayerConfig::fromYAML, "", py::arg("yamlString"));
}
