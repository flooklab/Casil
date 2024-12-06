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

#include <casil/HL/Direct/scpi.h>

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

using casil::HL::SCPI;

void bindHL_SCPI(py::module& pM)
{
    py::class_<SCPI, casil::HL::DirectDriver>(pM, "SCPI", "Driver for Standard Commands for Programmable Instruments (SCPI) devices.")
            .def(py::init<std::string, SCPI::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("__getattr__", [](const SCPI& pThis, const std::string_view pAttr) -> py::cpp_function
                                {
                                    const SCPI *const tThis = &pThis;   //Need to be pedantic and capture pointer by value
                                    return py::cpp_function(
                                        [tThis, pAttr](const std::optional<int> pChannel, SCPI::VariantValueType pValue)
                                            -> std::optional<std::string>
                                        { return tThis->operator()(pAttr, pChannel, std::move(pValue)); },
                                        py::arg("channel") = std::optional<int>{}, py::arg("value") = std::monostate{});
                                },
                 "Get a function to execute a command (either write or query; according return type).", py::arg("attr"), py::is_operator())
            .def("__call__", &SCPI::operator(), "Execute a command (either write or query).",
                 py::arg("cmd"), py::arg("channel") = std::nullopt, py::arg("value") = std::monostate{}, py::is_operator())
            .def("writeCommand", &SCPI::writeCommand, "Execute a write command.",
                 py::arg("cmd"), py::arg("channel") = std::nullopt, py::arg("value") = std::monostate{})
            .def("queryCommand", &SCPI::queryCommand, "Execute a query command.",
                 py::arg("cmd"), py::arg("channel") = std::nullopt)
            .def("command", &SCPI::command, "Execute a command (either write or query).",
                 py::arg("cmd"), py::arg("channel") = std::nullopt, py::arg("value") = std::monostate{});
}
