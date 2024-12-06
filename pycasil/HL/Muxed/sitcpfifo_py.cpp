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

#include <casil/HL/Muxed/sitcpfifo.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

using casil::HL::SiTCPFifo;

void bindHL_SiTCPFifo(py::module& pM)
{
    py::class_<SiTCPFifo, casil::HL::MuxedDriver>(pM, "SiTCPFifo", "Special driver to access the FIFO of the SiTCP interface.")
            .def(py::init<std::string, SiTCPFifo::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("__getitem__", [](SiTCPFifo& pThis, const std::string_view pItem) -> std::size_t
                                {
                                    if (pItem == "RESET")
                                    {
                                        pThis.reset();
                                        return std::size_t{0};
                                    }
                                    else if (pItem == "VERSION")
                                        return pThis.getVersion();
                                    else if (pItem == "FIFO_SIZE")
                                        return pThis.getFifoSize();
                                    else
                                        throw py::key_error("Invalid item \"" + std::string(pItem) + "\".");
                                }, "Register-like access to some functions (RESET, VERSION, FIFO_SIZE).", py::arg("item"), py::is_operator())
            .def("__setitem__", [](SiTCPFifo& pThis, const std::string_view pItem, std::uint8_t) -> void
                                {
                                    if (pItem == "RESET")
                                        pThis.reset();
                                    else
                                        throw py::key_error("Invalid item \"" + std::string(pItem) + "\".");
                                }, "Reset the FIFO via \"RESET\" (will fail otherwise).", py::arg("item"), py::arg("arg"), py::is_operator())
            .def("__setitem__", [](const SiTCPFifo&, const std::string_view pItem, py::object) -> void
                                {
                                    if (pItem == "RESET")
                                        throw py::type_error("Invalid assignment.");
                                    else
                                        throw py::key_error("Invalid item \"" + std::string(pItem) + "\".");
                                }, "This one will fail.", py::arg("item"), py::arg("arg"), py::is_operator())
            .def("__getattr__", [](SiTCPFifo& pThis, const std::string_view pAttr) -> std::variant<std::size_t, py::cpp_function>
                                {
                                    if (pAttr.starts_with("get_") && pAttr.length() >= 5 && (pAttr.substr(4) == "RESET" ||
                                                                                             pAttr.substr(4) == "VERSION" ||
                                                                                             pAttr.substr(4) == "FIFO_SIZE")
                                            )
                                    {
                                        SiTCPFifo *const tThis = &pThis;    //Need to be pedantic and capture pointer by value
                                        return py::cpp_function(
                                            [tThis, pAttr]() -> std::size_t
                                            {
                                                return tThis->operator[](pAttr.substr(4));
                                            }
                                        );
                                    }
                                    else if (pAttr.starts_with("set_") && pAttr.substr(4) == "RESET")
                                    {
                                        SiTCPFifo *const tThis = &pThis;    //Need to be pedantic and capture pointer by value
                                        return py::cpp_function(
                                            [tThis](std::uint8_t) -> void
                                            {
                                                tThis->reset();
                                            }
                                        );
                                    }
                                    else if (pAttr == "RESET" || pAttr == "VERSION" || pAttr == "FIFO_SIZE")
                                    {
                                        if (pAttr == "RESET")
                                        {
                                            pThis.reset();
                                            return std::size_t{0};
                                        }
                                        else if (pAttr == "VERSION")
                                            return pThis.getVersion();
                                        else
                                            return pThis.getFifoSize();
                                    }
                                    else
                                        throw py::attribute_error("Invalid attribute \"" + std::string(pAttr) + "\".");

                                },
                 "Get a getter function for RESET/VERSION/FIFO_SIZE, a setter function for RESET or access these register-like attributes "
                 "as with __getitem__.", py::arg("attr"), py::is_operator())
            .def("__setattr__", [](SiTCPFifo& pThis, const std::string_view pAttr, const std::uint8_t pValue) -> void
                                {
                                    if (pAttr == "RESET")
                                        pThis.reset();
                                    else
                                    {
                                        py::type selfType = py::type::of<SiTCPFifo>();
                                        py::object selfObj = py::cast(pThis);

                                        py::module::import("builtins").attr("super")(selfType, selfObj).attr("__setattr__")(pAttr, pValue);
                                    }
                                },
                 "Reset the FIFO via \"RESET\" or set a non-register attribute.", py::arg("attr"), py::arg("value"), py::is_operator())
            .def("__setattr__", [](SiTCPFifo& pThis, const std::string_view pAttr, const py::object pArg)   //Catchall forward to super
                                    -> void
                                {
                                    if (pAttr == "RESET")
                                        throw py::type_error("Invalid assignment.");
                                    else
                                    {
                                        py::type selfType = py::type::of<SiTCPFifo>();
                                        py::object selfObj = py::cast(pThis);

                                        py::module::import("builtins").attr("super")(selfType, selfObj).attr("__setattr__")(pAttr, pArg);
                                    }
                                }, "Set a non-register attribute.", py::arg("attr"), py::arg("arg"), py::is_operator())
            .def("getVersion", &SiTCPFifo::getVersion, "Get the pseudo FIFO module version.")
            .def("getFifoSize", &SiTCPFifo::getFifoSize, "Get the FIFO size in number of bytes.")
            .def("getFifoData", &SiTCPFifo::getFifoData, "Read the FIFO content as sequence of 32 bit unsigned integers.")
            .def("setFifoData", &SiTCPFifo::setFifoData, "Write a sequence of 32 bit unsigned integers to the FIFO.", py::arg("data"));
}
