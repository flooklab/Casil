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

#include <casil/HL/Muxed/i2c.h>

#include <boost/dynamic_bitset.hpp>

#include <bitset>
#include <cstdint>
#include <stdexcept>
#include <vector>

using casil::HL::I2C;

void bindHL_I2C(py::module& pM)
{
    py::class_<I2C, casil::HL::RegisterDriver>(pM, "I2C", "Driver for the i2c firmware module.")
            .def(py::init<std::string, I2C::InterfaceBaseType&, casil::LayerConfig>(), "Constructor.",
                 py::arg("name"), py::arg("interface"), py::arg("config"))
            .def("getData", &I2C::getData, "Get the content of the transaction memory.", py::arg("size") = -1, py::arg("addrOffs") = 0u)
            .def("setData", &I2C::setData, "Set the content of the transaction memory.", py::arg("data"), py::arg("addrOffs") = 0u)
            .def("exec", &I2C::exec, "Start the transaction.")
            .def("isDone", &I2C::isDone, "Check if the transaction was completed.")
            .def("start", &I2C::start, "Start the transaction.")
            .def("isReady", &I2C::isReady, "Check if the transaction was completed.")
            .def("memSize", &I2C::memSize, "Get the MEM_BYTES register.")
            .def("getAddr", &I2C::getAddr, "Get the ADDR register.")
            .def("setAddr", &I2C::setAddr, "Set the ADDR register.")
            .def("getSlaveAddr", [](const I2C& pThis) -> std::vector<bool>
                                 { return PyCasilUtils::boolVecFromBitset(boost::dynamic_bitset(7, pThis.getSlaveAddr().to_ulong())); },
                 "Get slave address from ADDR register.")
            .def("setSlaveAddr", [](I2C& pThis, const std::vector<bool>& pAddr) -> void
                                 {
                                     if (pAddr.size() != 7)
                                         throw std::invalid_argument("Invalid I2C address: Must be 7 bits long.");
                                     pThis.setSlaveAddr(std::bitset<7>(PyCasilUtils::bitsetFromBoolVec(pAddr).to_ulong()));
                                 },
                 "Set slave address for ADDR register.", py::arg("addr"))
            .def("getReadWriteBit", &I2C::getReadWriteBit, "Get read/write bit from ADDR register.")
            .def("setReadWriteBit", &I2C::setReadWriteBit, "Set read/write bit for ADDR register.", py::arg("read"))
            .def("getSize", &I2C::getSize, "Get the SIZE register.")
            .def("setSize", &I2C::setSize, "Set the SIZE register.", py::arg("size"))
            .def("sendRead", [](I2C& pThis, const std::uint8_t pAddr, const int pSize) -> std::vector<std::uint8_t>
                             { return pThis.sendRead(pAddr, pSize); },
                 "Configure and start a read transaction.", py::arg("addr"), py::arg("size") = -1)
            .def("sendRead", [](I2C& pThis, const std::vector<bool>& pAddr, const int pSize) -> std::vector<std::uint8_t>
                             {
                                 if (pAddr.size() != 7)
                                     throw std::invalid_argument("Invalid I2C address: Must be 7 bits long.");
                                 return pThis.sendRead(std::bitset<7>(PyCasilUtils::bitsetFromBoolVec(pAddr).to_ulong()), pSize);
                             }, "Configure and start a read transaction.", py::arg("addr"), py::arg("size") = -1)
            .def("sendWrite", [](I2C& pThis, const std::uint8_t pAddr, const std::vector<std::uint8_t>& pData) -> void
                              { pThis.sendWrite(pAddr, pData); },
                 "Configure and start a write transaction.", py::arg("addr"), py::arg("data"))
            .def("sendWrite", [](I2C& pThis, const std::vector<bool>& pAddr, const std::vector<std::uint8_t>& pData) -> void
                              {
                                  if (pAddr.size() != 7)
                                      throw std::invalid_argument("Invalid I2C address: Must be 7 bits long.");
                                  pThis.sendWrite(std::bitset<7>(PyCasilUtils::bitsetFromBoolVec(pAddr).to_ulong()), pData);
                              }, "Configure and start a write transaction.", py::arg("addr"), py::arg("data"));
}
