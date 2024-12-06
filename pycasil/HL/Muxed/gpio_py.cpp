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

#include <casil/HL/Muxed/gpio.h>

#include <boost/dynamic_bitset.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

using casil::HL::GPIO;

void bindHL_GPIO(py::module& pM)
{
    py::class_<GPIO, casil::HL::RegisterDriver>(pM, "GPIO", "Driver for the gpio firmware module.")
            .def(py::init<std::string, GPIO::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("getSize", &GPIO::getSize, "Get the number of IO bits.")
            .def("getOutputEn", &GPIO::getOutputEn, "Get the OUTPUT_EN register.")
            .def("setOutputEn", &GPIO::setOutputEn, "Set the OUTPUT_EN register.", py::arg("enable"))
            .def("bitsetFromBytes", [](const GPIO& pThis, const std::vector<std::uint8_t>& pBytes) -> std::vector<bool>
                                    {
                                        const boost::dynamic_bitset bitset = pThis.bitsetFromBytes(pBytes);
                                        std::vector<bool> retVal(bitset.size(), false);
                                        for (std::size_t i = 0; i < bitset.size(); ++i)
                                            retVal[i] = bitset.test(bitset.size() - 1 - i);
                                        return retVal;
                                    }, "Convert IO register bytes to a bitset.", py::arg("bytes"))
            .def("bytesFromBitset", [](const GPIO& pThis, const std::vector<bool>& pBits) -> std::vector<std::uint8_t>
                                    {
                                        boost::dynamic_bitset bitset(pBits.size());
                                        for (std::size_t i = 0; i < pBits.size(); ++i)
                                            bitset[i] = pBits[pBits.size() - 1 - i];
                                        return pThis.bytesFromBitset(bitset);
                                    }, "Convert a bitset to IO register bytes.", py::arg("bits"));
}
