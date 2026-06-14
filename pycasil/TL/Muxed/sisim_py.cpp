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

#include <casil/TL/Muxed/sisim.h>

using casil::TL::SiSim;

void bindTL_SiSim(py::module& pM)
{
    py::class_<SiSim, casil::TL::MuxedInterface> siSim(pM, "SiSim",
                                                       "Interface to virtually connect to the basil bus in a cocotb co-simulation.");

    py::native_enum<SiSim::MessageType>(siSim, "MessageType", "enum.Enum",
                                        "Type of a message that is passed between interface/simulation or vice versa.")
            .value("WriteRequest", SiSim::MessageType::WriteRequest, "Write request from interface to simulation.")
            .value("ReadRequest", SiSim::MessageType::ReadRequest, "Read request from interface to simulation.")
            .value("ReadResponse", SiSim::MessageType::ReadResponse, "Read response from simulation back to interface.")
            .finalize();

    siSim.def(py::init<std::string, casil::LayerConfig>(), "Constructor.", py::arg("name"), py::arg("config"))
            //.def_static("createWriteRequest", &SiSim::createWriteRequest,
            //            "Create a write request to send to simulation.", py::arg("addr"), py::arg("data"))    //Not really needed in PyCasil
            //.def_static("createReadRequest", &SiSim::createReadRequest,
            //            "Create a read request to send to simulation.", py::arg("addr"), py::arg("size"))     //Not really needed in PyCasil
            .def_static("createReadResponse", &SiSim::createReadResponse, "Create a read response to send back to interface.",
                        py::arg("data"))
            .def_static("parseWriteRequest", &SiSim::parseWriteRequest, "Parse a write request as received from interface.", py::arg("data"))
            .def_static("parseReadRequest", &SiSim::parseReadRequest, "Parse a read request as received from interface.", py::arg("data"))
            //.def_static("parseReadResponse", &SiSim::parseReadResponse,
            //            "Parse a read response as received from simulation.", py::arg("data"))                //Not really needed in PyCasil
            .def_static("parseMessageType", &SiSim::parseMessageType, "Parse message type from first message byte.", py::arg("typeByte"))
            .def_static("parseMessageSize", &SiSim::parseMessageSize, "Parse payload size from message size bytes.", py::arg("data"));
}
