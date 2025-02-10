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

#include <casil/TL/Muxed/sitcp.h>

using casil::TL::SiTCP;

void bindTL_SiTCP(py::module& pM)
{
    py::class_<SiTCP, casil::TL::MuxedInterface>(pM, "SiTCP", "Interface to connect to the basil bus on an FPGA "
                                                              "that runs the SiTCP library for communication.")
            .def(py::init<std::string, casil::LayerConfig>(), "Constructor.", py::arg("name"), py::arg("config"))
            .def("readBufferEmpty", &SiTCP::readBufferEmpty, "Check if the UDP read buffer is empty.")
            .def("clearReadBuffer", &SiTCP::clearReadBuffer, "Clear the current contents of the UDP read buffer.")
            .def("resetFifo", &SiTCP::resetFifo, "Clear the FIFO and the remaining incoming %TCP buffer.")
            .def("getFifoSize", &SiTCP::getFifoSize, "Get the FIFO size in number of bytes.")
            .def("getFifoData", &SiTCP::getFifoData, "Extract the current FIFO content as sequence of bytes.", py::arg("size") = -1)
            .def_readonly_static("baseAddrDataLimit", &SiTCP::baseAddrDataLimit,
                                 "Address limit below which read() / write() do normal bus access.")
            .def_readonly_static("baseAddrFIFOLimit", &SiTCP::baseAddrFIFOLimit,
                                 "Address limit for special FIFO access of read() / write().");
}
