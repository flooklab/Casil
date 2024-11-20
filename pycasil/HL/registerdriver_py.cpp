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

#include <casil/HL/registerdriver.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using casil::HL::RegisterDriver;

void bindHL_RegisterDriver(py::module& pM)
{
    py::class_<RegisterDriver, casil::HL::MuxedDriver>(pM, "RegisterDriver", "")
            .def("__getitem__", &RegisterDriver::get, "", py::arg("regName"), py::is_operator())
            .def("__setitem__", [](RegisterDriver& pThis, const std::string_view pRegName, const std::uint64_t pValue) -> void
                                { pThis.setValue(pRegName, pValue); }, "", py::arg("regName"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](RegisterDriver& pThis, const std::string_view pRegName, const std::vector<std::uint8_t>& pBytes) -> void
                                { pThis.setBytes(pRegName, pBytes); }, "", py::arg("regName"), py::arg("bytes"), py::is_operator())
            .def("__setitem__", [](const RegisterDriver& pThis, const std::string_view pRegName, py::object) -> void
                                {
                                    try
                                    {
                                        if (pThis.testRegisterName(pRegName))
                                            throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "", py::arg("regName"), py::arg("arg"), py::is_operator())
            .def("__getattr__", [](RegisterDriver& pThis, const std::string_view pAttr) -> std::variant<std::uint64_t,
                                                                                                        std::vector<std::uint8_t>,
                                                                                                        py::cpp_function>
                                {
                                    if (pAttr.starts_with("get_") && pAttr.length() >= 5 &&
                                        RegisterDriver::isValidRegisterName(pAttr.substr(4)))
                                    {
                                        RegisterDriver *const tThis = &pThis;   //Need to be pedantic and capture pointer by value
                                        return py::cpp_function(
                                            [tThis, pAttr]() -> std::variant<std::uint64_t, std::vector<std::uint8_t>>
                                            { return tThis->get(pAttr.substr(4)); }
                                        );
                                    }
                                    else if (pAttr.starts_with("set_") && pAttr.length() >= 5 &&
                                             RegisterDriver::isValidRegisterName(pAttr.substr(4)))
                                    {
                                        RegisterDriver *const tThis = &pThis;   //Need to be pedantic and capture pointer by value
                                        return py::cpp_function(
                                            [tThis, pAttr](const std::variant<std::uint64_t, std::vector<std::uint8_t>> pValue)
                                                -> void
                                            {
                                                if (std::holds_alternative<std::uint64_t>(pValue))
                                                    tThis->set(pAttr.substr(4), std::get<std::uint64_t>(pValue));
                                                else
                                                    tThis->set(pAttr.substr(4), std::get<std::vector<std::uint8_t>>(pValue));
                                            }
                                        );
                                    }
                                    else if (RegisterDriver::isValidRegisterName(pAttr))
                                    {
                                        try
                                        {
                                            std::variant<std::uint64_t, std::vector<std::uint8_t>> regVal = pThis.get(pAttr);

                                            if (std::holds_alternative<std::uint64_t>(regVal))
                                                return std::get<std::uint64_t>(regVal);
                                            else
                                                return std::get<std::vector<std::uint8_t>>(regVal);
                                        }
                                        catch (const std::invalid_argument&)
                                        {
                                            throw;
                                        }
                                    }
                                    else
                                        throw py::attribute_error("Invalid attribute \"" + std::string(pAttr) + "\".");

                                }, "", py::arg("attr"), py::is_operator())
            .def("__setattr__", [](RegisterDriver& pThis, const std::string_view pAttr, const std::uint64_t pValue) -> void
                                {
                                    if (RegisterDriver::isValidRegisterName(pAttr))
                                    {
                                        try
                                        {
                                            pThis.setValue(pAttr, pValue);
                                            return;
                                        }
                                        catch (const std::invalid_argument&)
                                        {
                                            throw;
                                        }
                                    }
                                    else
                                    {
                                        py::type selfType = py::type::of<RegisterDriver>();
                                        py::object selfObj = py::cast(pThis);

                                        py::module::import("builtins").attr("super")(selfType, selfObj).attr("__setattr__")(pAttr, pValue);
                                    }
                                }, "", py::arg("attr"), py::arg("value"), py::is_operator())
            .def("__setattr__", [](RegisterDriver& pThis, const std::string_view pAttr, const std::vector<std::uint8_t>& pBytes) -> void
                                {
                                    if (RegisterDriver::isValidRegisterName(pAttr))
                                    {
                                        try
                                        {
                                            pThis.setBytes(pAttr, pBytes);
                                            return;
                                        }
                                        catch (const std::invalid_argument&)
                                        {
                                            throw;
                                        }
                                    }
                                    else
                                    {
                                        py::type selfType = py::type::of<RegisterDriver>();
                                        py::object selfObj = py::cast(pThis);

                                        py::module::import("builtins").attr("super")(selfType, selfObj).attr("__setattr__")(pAttr, pBytes);
                                    }
                                }, "", py::arg("attr"), py::arg("bytes"), py::is_operator())
            .def("__setattr__", [](RegisterDriver& pThis, const std::string_view pAttr, const py::object pArg)      //Catchall forward to super
                                    -> void
                                {
                                    if (RegisterDriver::isValidRegisterName(pAttr))
                                    {
                                        try
                                        {
                                            if (pThis.testRegisterName(pAttr))
                                                throw py::type_error("Invalid assignment.");
                                        }
                                        catch (const std::invalid_argument&)
                                        {
                                            throw;
                                        }
                                    }
                                    else
                                    {
                                        py::type selfType = py::type::of<RegisterDriver>();
                                        py::object selfObj = py::cast(pThis);

                                        py::module::import("builtins").attr("super")(selfType, selfObj).attr("__setattr__")(pAttr, pArg);
                                    }
                                }, "", py::arg("attr"), py::arg("arg"), py::is_operator())
            .def("applyDefaults", &RegisterDriver::applyDefaults, "")
            .def("getBytes", &RegisterDriver::getBytes, "", py::arg("regName"))
            .def("setBytes", &RegisterDriver::setBytes, "", py::arg("regName"), py::arg("data"))
            .def("getValue", &RegisterDriver::getValue, "", py::arg("regName"))
            .def("setValue", &RegisterDriver::setValue, "", py::arg("regName"), py::arg("value"))
            .def("get", &RegisterDriver::get, "", py::arg("regName"))
            .def("set", [](RegisterDriver& pThis, const std::string_view pRegName, const std::uint64_t pValue) -> void
                        { pThis.setValue(pRegName, pValue); }, "", py::arg("regName"), py::arg("value"))
            .def("set", [](RegisterDriver& pThis, const std::string_view pRegName, const std::vector<std::uint8_t>& pBytes) -> void
                        { pThis.setBytes(pRegName, pBytes); }, "", py::arg("regName"), py::arg("bytes"))
            .def("trigger", &RegisterDriver::trigger, "", py::arg("regName"))
            .def("testRegisterName", &RegisterDriver::testRegisterName, "", py::arg("regName"))
            .def_static("isValidRegisterName", &RegisterDriver::isValidRegisterName, "", py::arg("regName"));
}
