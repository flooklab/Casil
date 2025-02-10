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

#include <casil/layerconfig.h>

using casil::LayerConfig;

void bind_LayerConfig(py::module& pM)
{
    py::class_<LayerConfig>(pM, "LayerConfig", "Configuration object for layer components derived from LayerBase.")
            .def(py::init<>(), "Default constructor.")
            .def(py::init<const LayerConfig&>(), "Default copy constructor.", py::arg("other"))
            .def("__eq__", (&LayerConfig::operator==), "Equality operator.", py::arg("other"), py::is_operator())
            .def("contains", &LayerConfig::contains, "Check the configuration tree structure (and value types).",
                 py::arg("other"), py::arg("checkTypes") = false)
            .def("getBool", &LayerConfig::getBool, "Get a boolean configuration value.", py::arg("key"), py::arg("default") = false)
            .def("getInt", &LayerConfig::getInt, "Get a (signed) integer configuration value.", py::arg("key"), py::arg("default") = 0)
            .def("getUInt", &LayerConfig::getUInt, "Get an unsigned integer configuration value.", py::arg("key"), py::arg("default") = 0u)
            .def("getDbl", &LayerConfig::getDbl, "Get a floating point configuration value.", py::arg("key"), py::arg("default") = 0.0)
            .def("getStr", &LayerConfig::getStr, "Get a string-type configuration value.", py::arg("key"), py::arg("default") = "")
            .def("getByteSeq", &LayerConfig::getByteSeq, "Get an 8 bit unsigned integer sequence from the configuration tree.",
                 py::arg("key"), py::arg("default") = std::vector<std::uint8_t>{})
            .def("getUIntSeq", &LayerConfig::getUIntSeq, "Get a 64 bit unsigned integer sequence from the configuration tree.",
                 py::arg("key"), py::arg("default") = std::vector<std::uint64_t>{})
            .def_static("fromYAML", &LayerConfig::fromYAML, "Create a configuration object from YAML format.", py::arg("yamlString"));
}
