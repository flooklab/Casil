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

#include <casil/bytes.h>

#include <boost/dynamic_bitset.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Bytes = casil::Bytes;

void bindBytes(py::module& pM)
{
    pM.def("composeUInt16", [](const std::vector<std::uint8_t>& pBytes, const bool pBigEndian) -> std::uint16_t
                            { return Bytes::composeUInt16(pBytes, pBigEndian); }, "", py::arg("bytes"), py::arg("bigEndian") = true);
    pM.def("composeUInt32", [](const std::vector<std::uint8_t>& pBytes, const bool pBigEndian) -> std::uint32_t
                            { return Bytes::composeUInt32(pBytes, pBigEndian); }, "", py::arg("bytes"), py::arg("bigEndian") = true);
    pM.def("composeUInt64", [](const std::vector<std::uint8_t>& pBytes, const bool pBigEndian) -> std::uint64_t
                            { return Bytes::composeUInt64(pBytes, pBigEndian); }, "", py::arg("bytes"), py::arg("bigEndian") = true);

    pM.def("bitsetFromBytes", [](const std::vector<std::uint8_t>& pBytes, const std::size_t pBitSize) -> std::vector<bool>
                              {
                                  const boost::dynamic_bitset bitset = Bytes::bitsetFromBytes(pBytes, pBitSize);
                                  std::vector<bool> retVal(bitset.size(), false);
                                  for (std::size_t i = 0; i < bitset.size(); ++i)
                                      retVal[i] = bitset.test(bitset.size() - 1 - i);
                                  return retVal;
                              }, "", py::arg("bytes"), py::arg("bitSize"));
    pM.def("bytesFromBitset", [](const std::vector<bool>& pBits, const std::size_t pByteSize) -> std::vector<std::uint8_t>
                              {
                                  boost::dynamic_bitset bitset(pBits.size());
                                  for (std::size_t i = 0; i < pBits.size(); ++i)
                                      bitset[i] = pBits[pBits.size() - 1 - i];
                                  return Bytes::bytesFromBitset(bitset, pByteSize);
                              }, "", py::arg("bits"), py::arg("byteSize"));

    pM.def("byteVecFromStr", &Bytes::byteVecFromStr, "", py::arg("str"));
    pM.def("strFromByteVec", &Bytes::strFromByteVec, "", py::arg("vec"));

    pM.def("formatHex", [](const std::uint64_t pNumber) -> std::string { return Bytes::formatHex<std::uint64_t>(pNumber, false); }, "",
           py::arg("number"));
    pM.def("formatByteVec", &Bytes::formatByteVec, "", py::arg("vec"));
    pM.def("formatUIntVec", &Bytes::formatUInt64Vec, "", py::arg("vec"));
}
