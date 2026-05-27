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

#include <casil/HL/Muxed/spi.h>

#include <cstdint>
#include <vector>

using casil::HL::SPI;

void bindHL_SPI(py::module& pM)
{
    py::class_<SPI, casil::HL::RegisterDriver>(pM, "SPI", "Driver for the spi firmware module.")
            .def(py::init<std::string, SPI::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("getData", &SPI::getData, "Get the content of the transaction memory.", py::arg("size") = -1, py::arg("addrOffs") = 0u)
            .def("setData", &SPI::setData, "Set the content of the transaction memory.", py::arg("data"), py::arg("addrOffs") = 0u)
            .def("exec", &SPI::exec, "Start the transaction.")
            .def("isDone", &SPI::isDone, "Check if the transaction was completed.")
            .def("start", &SPI::start, "Start the transaction.")
            .def("isReady", &SPI::isReady, "Check if the transaction was completed.")
            .def("memSize", &SPI::memSize, "Get the MEM_BYTES register.")
            .def("getSize", &SPI::getSize, "Get the SIZE register.")
            .def("setSize", &SPI::setSize, "Set the SIZE register.", py::arg("size"))
            .def("getWait", &SPI::getWait, "Get the WAIT register.")
            .def("setWait", &SPI::setWait, "Set the WAIT register.", py::arg("cycles"))
            .def("getRepeat", &SPI::getRepeat, "Get the REPEAT register.")
            .def("setRepeat", &SPI::setRepeat, "Set the REPEAT register.", py::arg("reps"))
            .def("getEn", &SPI::getEn, "Get the EN register.")
            .def("setEn", &SPI::setEn, "Set the EN register.", py::arg("enable"))
            .def("sendCmd", [](SPI& pThis, const std::vector<std::uint8_t>& pCmd) -> void
                             { pThis.sendCmd(pCmd); }, "Configure and start a transaction.", py::arg("cmd"))
            .def("sendCmd", [](SPI& pThis, const std::uint8_t pCmd) -> void
                             { pThis.sendCmd(pCmd); }, "Configure and start a transaction.", py::arg("cmd"))
            .def("sendCmd", [](SPI& pThis, const std::uint8_t pCmdMsb, const std::uint8_t pCmdLsb) -> void
                             { pThis.sendCmd(pCmdMsb, pCmdLsb); },
                 "Configure and start a transaction.", py::arg("cmdMsb"), py::arg("cmdLsb"))
            .def("sendCmdRead", [](SPI& pThis, const std::vector<std::uint8_t>& pCmd, const int pSize) -> std::vector<std::uint8_t>
                                { return pThis.sendCmdRead(pCmd, pSize); },
                 "Configure and start a transaction and get the readback.", py::arg("cmd"), py::arg("size") = -1)
            .def("sendCmdRead", [](SPI& pThis, const std::uint8_t pCmd) -> std::vector<std::uint8_t>
                                { return pThis.sendCmdRead(pCmd); },
                 "Configure and start a transaction and get the readback.", py::arg("cmd"))
            .def("sendCmdRead", [](SPI& pThis, const std::uint8_t pCmdMsb, const std::uint8_t pCmdLsb) -> std::vector<std::uint8_t>
                                { return pThis.sendCmdRead(pCmdMsb, pCmdLsb); },
                 "Configure and start a transaction and get the readback.", py::arg("cmdMsb"), py::arg("cmdLsb"));
}
