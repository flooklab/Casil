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

#include <casil/HL/Direct/tti_ql355tp.h>

using casil::HL::TTiQL355TP;

void bindHL_TTiQL355TP(py::module& pM)
{
    py::class_<TTiQL355TP, casil::HL::DirectDriver>(pM, "TTiQL355TP", "Driver for the Aim-TTi QL355TP power supply.")
            .def(py::init<std::string, TTiQL355TP::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("reInitIntf", &TTiQL355TP::reInitIntf, "Re-initialize underlying interface instance.")
            .def("writeCmd", &TTiQL355TP::writeCmd, "Execute an arbitrary write command.", py::arg("cmd"))
            .def("queryCmd", &TTiQL355TP::queryCmd, "Execute an arbitrary query command.", py::arg("cmd"))
            .def("getDeviceIdentifier", &TTiQL355TP::getDeviceIdentifier, "Get the SCPI device identifier.")
            .def("setEnable", &TTiQL355TP::setEnable, "Enable/disable one or all output channels.", py::arg("on"), py::arg("channel"))
            .def("getVoltage", &TTiQL355TP::getVoltage, "Get the measured output voltage of a channel.", py::arg("channel"))
            .def("getSetVoltage", &TTiQL355TP::getSetVoltage, "Get the set output voltage of a channel.", py::arg("channel"))
            .def("setVoltage", &TTiQL355TP::setVoltage, "Set the output voltage of a channel.", py::arg("value"), py::arg("channel"))
            .def("getCurrent", &TTiQL355TP::getCurrent, "Get the measured output current of a channel.", py::arg("channel"))
            .def("getCurrentLimit", &TTiQL355TP::getCurrentLimit, "Get the set current limit of a channel.", py::arg("channel"))
            .def("setCurrentLimit", &TTiQL355TP::setCurrentLimit, "Set the current limit of a channel.", py::arg("value"), py::arg("channel"))
            .def("resetTrip", &TTiQL355TP::resetTrip, "Reset the OVP and OCP trips.");
}
