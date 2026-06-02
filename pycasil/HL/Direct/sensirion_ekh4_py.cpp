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

#include <casil/HL/Direct/sensirion_ekh4.h>

using casil::HL::SensirionEKH4;

void bindHL_SensirionEKH4(py::module& pM)
{
    py::class_<SensirionEKH4, casil::HL::DirectDriver>(pM, "SensirionEKH4",
                                                       "Driver for the Sensirion EK-H4 temperature/humidity sensor multiplexer box.")
            .def(py::init<std::string, SensirionEKH4::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("getTemperatures", &SensirionEKH4::getTemperatures, "Get the temperature values from all four sensors.")
            .def("getHumidities", &SensirionEKH4::getHumidities, "Get the humidity values from all four sensors.")
            .def("getDewPoints", &SensirionEKH4::getDewPoints, "Get the dew point values from all four sensors.")
            .def("getTemperature", &SensirionEKH4::getTemperature, "Get the temperature value of one of the sensors.", py::arg("channel"))
            .def("getHumidity", &SensirionEKH4::getHumidity, "Get the humidity value of one of the sensors.", py::arg("channel"))
            .def("getDewPoint", &SensirionEKH4::getDewPoint, "Get the dew point value of one of the sensors.", py::arg("channel"));
}
