/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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

#include <casil/RL/standardregister.h>

#include <boost/dynamic_bitset.hpp>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using casil::RL::StandardRegister;
using RegField = StandardRegister::RegField;

void bindRL_StandardRegister(py::module& pM)
{
    py::class_<RegField>(pM, "RegField", "Proxy class for accessing an individual register field.")
            .def("__getitem__", [](const RegField& pThis, const std::size_t pIdx) -> bool
                                { return pThis.operator[](pIdx).get(); }, "Access a specific bit in the field.",
                                py::arg("idx"), py::is_operator())
            .def("__getitem__", [](RegField& pThis, const std::string_view pFieldName) -> RegField&
                                { return pThis.operator[](pFieldName); }, "Access an immediate child field.",
                                py::arg("fieldName"), py::return_value_policy::reference, py::is_operator())
            .def("__getitem__", [](RegField& pThis, const std::size_t pMsbIdx, const std::size_t pLsbIdx) -> RegField
                                { return pThis.operator()(pMsbIdx, pLsbIdx); }, "Access a slice of bits in the field.",
                                py::arg("msbIdx"), py::arg("lsbIdx"), py::is_operator())
            .def("__getitem__", [](RegField& pThis, const std::vector<std::size_t>& pIdxs) -> RegField
                                { return pThis.operator[](pIdxs); }, "Access a set of unique bits in the field.",
                                py::arg("idxs"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::size_t pIdx, const bool pValue) -> void
                                { pThis.operator[](pIdx) = pValue; }, "Assign a boolean value to a specific bit in the field.",
                                py::arg("idx"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](const RegField& pThis, const std::size_t pIdx, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator[](pIdx);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to a field bit (will fail).",
                                py::arg("idx"), py::arg("arg"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::string_view pFieldName, const std::uint64_t pValue) -> void
                                { pThis.operator[](pFieldName) = pValue; }, "Assign an integer value to an immediate child field.",
                                py::arg("fieldName"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::string_view pFieldName, const std::vector<bool>& pBits) -> void
                                { pThis.operator[](pFieldName) = PyCasilUtils::bitsetFromBoolVec(pBits); },
                                "Assign a bit sequence to an immediate child field.", py::arg("fieldName"), py::arg("bits"), py::is_operator())
            .def("__setitem__", [](const RegField& pThis, const std::string_view pFieldName, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator[](pFieldName);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to an immediate child field (will fail).",
                                py::arg("fieldName"), py::arg("arg"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::size_t pMsbIdx, const std::size_t pLsbIdx, const std::uint64_t pValue)
                                    -> void
                                { pThis.operator()(pMsbIdx, pLsbIdx) = pValue; }, "Assign an integer value to a slice of bits in the field.",
                                py::arg("msbIdx"), py::arg("lsbIdx"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::size_t pMsbIdx, const std::size_t pLsbIdx, const std::vector<bool>& pBits)
                                    -> void
                                { pThis.operator()(pMsbIdx, pLsbIdx) = PyCasilUtils::bitsetFromBoolVec(pBits); },
                                "Assign a bit sequence to a slice of bits in the field.",
                                py::arg("msbIdx"), py::arg("lsbIdx"), py::arg("bits"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::size_t pMsbIdx, const std::size_t pLsbIdx, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator()(pMsbIdx, pLsbIdx);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to a slice of bits in the field (will fail).",
                                py::arg("msbIdx"), py::arg("lsbIdx"), py::arg("arg"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::vector<std::size_t>& pIdxs, const std::uint64_t pValue) -> void
                                { pThis.operator[](pIdxs) = pValue; }, "Assign an integer value to a set of unique bits in the field.",
                                py::arg("idxs"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::vector<std::size_t>& pIdxs, const std::vector<bool>& pBits) -> void
                                { pThis.operator[](pIdxs) = PyCasilUtils::bitsetFromBoolVec(pBits); },
                                "Assign a bit sequence to a set of unique bits in the field.",
                                py::arg("idxs"), py::arg("bits"), py::is_operator())
            .def("__setitem__", [](RegField& pThis, const std::vector<std::size_t>& pIdxs, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator[](pIdxs);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to a set of unique bits in the field (will fail).",
                                py::arg("idxs"), py::arg("arg"), py::is_operator())
            .def("set", [](RegField& pThis, const std::uint64_t pValue) -> void
                        { pThis.set(pValue); }, "Assign equivalent integer value to the field.", py::arg("value"))
            .def("set", [](RegField& pThis, const std::vector<bool>& pBits) -> void
                        { pThis.set(PyCasilUtils::bitsetFromBoolVec(pBits)); }, "Assign a raw bit sequence to the field.", py::arg("bits"))
            .def("setAll", &RegField::setAll, "Set/unset all field bits at once.", py::arg("value") = true)
            .def("toUInt", &RegField::toUInt, "Get the integer equivalent of field's content.")
            .def("toBits", [](const RegField& pThis) -> std::vector<bool>
                           { return PyCasilUtils::boolVecFromBitset(pThis.toBits()); }, "Get the field's data as raw bitset.")
            .def("n", [](RegField& pThis, const std::size_t pFieldRepIdx) -> RegField& { return pThis.n(pFieldRepIdx); },
                 "Access the n-th repetition of the field.", py::arg("fieldRepIdx"), py::return_value_policy::reference)
            .def("getSize", &RegField::getSize, "Get the size of the field.")
            .def("__len__", &RegField::getSize, "Get the size of the field.", py::is_operator())
            .def("getOffset", &RegField::getOffset, "Get the field's offset with respect to its parent field.")
            .def("getTotalOffset", &RegField::getTotalOffset, "Get the field's total offset with respect to the whole register.");

    py::class_<StandardRegister, casil::RL::Register>(pM, "StandardRegister", "... TODO ...")
            .def(py::init<std::string, casil::HL::Driver&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("driver"), py::arg("config"))
            .def("__getitem__", [](const StandardRegister& pThis, const std::size_t pIdx) -> bool
                                { return pThis.operator[](pIdx).get(); }, "Access a specific bit in the register.",
                                py::arg("idx"), py::is_operator())
            .def("__getitem__", [](StandardRegister& pThis, const std::string& pFieldPath) -> RegField&
                                { return pThis.operator[](pFieldPath); }, "Access a specific register field.", py::arg("fieldPath"),
                                py::return_value_policy::reference, py::is_operator())
            .def("__setitem__", [](StandardRegister& pThis, const std::size_t pIdx, const bool pValue) -> void
                                { pThis.operator[](pIdx) = pValue; }, "Assign a boolean value to a specific bit in the register.",
                                py::arg("idx"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](const StandardRegister& pThis, const std::size_t pIdx, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator[](pIdx);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to a register bit (will fail).",
                                py::arg("idx"), py::arg("arg"), py::is_operator())
            .def("__setitem__", [](StandardRegister& pThis, const std::string& pFieldPath, const std::uint64_t pValue) -> void
                                { pThis.operator[](pFieldPath) = pValue; }, "Assign an integer value to a field.",
                                py::arg("fieldPath"), py::arg("value"), py::is_operator())
            .def("__setitem__", [](StandardRegister& pThis, const std::string& pFieldPath, const std::vector<bool>& pBits) -> void
                                { pThis.operator[](pFieldPath) = PyCasilUtils::bitsetFromBoolVec(pBits); },
                                "Assign a bit sequence to a field.", py::arg("fieldPath"), py::arg("bits"), py::is_operator())
            .def("__setitem__", [](const StandardRegister& pThis, const std::string& pFieldPath, py::object) -> void
                                {
                                    try
                                    {
                                        (void)pThis.operator[](pFieldPath);
                                        throw py::type_error("Invalid assignment.");
                                    }
                                    catch (const std::invalid_argument&)
                                    {
                                        throw;
                                    }
                                }, "Assign something else to a register field (will fail).",
                                py::arg("fieldPath"), py::arg("arg"), py::is_operator())
            .def("root", [](StandardRegister& pThis) -> RegField& { return pThis.root(); }, "Get the root field node.",
                 py::return_value_policy::reference)
            .def("rootRead", &StandardRegister::rootRead, "Get the root field node for driver readback data.",
                 py::return_value_policy::reference)
            .def("getSize", &StandardRegister::getSize, "Get the size of the register.")
            .def("__len__", &StandardRegister::getSize, "Get the size of the register.", py::is_operator())
            .def("applyDefaults", &StandardRegister::applyDefaults, "Set register fields to configured default/init values.")
            .def("set", [](StandardRegister& pThis, const std::uint64_t pValue) -> void
                        { pThis.set(pValue); }, "Assign equivalent integer value to the register.")
            .def("set", [](StandardRegister& pThis, const std::vector<bool>& pBits) -> void
                        { pThis.set(PyCasilUtils::bitsetFromBoolVec(pBits)); }, "Assign a raw bit sequence to the register.")
            .def("setAll", &StandardRegister::setAll, "Set/unset all register bits at once.", py::arg("value") = true)
            .def("get", [](const StandardRegister& pThis) -> std::vector<bool>
                        { return PyCasilUtils::boolVecFromBitset(pThis.get()); }, "Get the register data as raw bit sequence.")
            .def("getRead", [](const StandardRegister& pThis) -> std::vector<bool>
                            { return PyCasilUtils::boolVecFromBitset(pThis.getRead()); }, "Get the driver readback data as a bit sequence.")
            .def("write", &StandardRegister::write, "Write the register data to the driver.", py::arg("numBytes") = 0)
            .def("read", &StandardRegister::read, "Read from the driver and assign to the readback data.", py::arg("numBytes") = 0)
            .def("toBytes", &StandardRegister::toBytes, "Convert the register data to a byte sequence.")
            .def("fromBytes", &StandardRegister::fromBytes, "Load/assign the register data from a byte sequence.");
}
