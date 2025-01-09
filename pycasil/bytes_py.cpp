/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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
                            { return Bytes::composeUInt16(pBytes, pBigEndian); },
           "Create a 16 bit unsigned integer from a two byte sequence of a certain endianness.",
           py::arg("bytes"), py::arg("bigEndian") = true);
    pM.def("composeUInt32", [](const std::vector<std::uint8_t>& pBytes, const bool pBigEndian) -> std::uint32_t
                            { return Bytes::composeUInt32(pBytes, pBigEndian); },
           "Create a 32 bit unsigned integer from a four byte sequence of a certain endianness.",
           py::arg("bytes"), py::arg("bigEndian") = true);
    pM.def("composeUInt64", [](const std::vector<std::uint8_t>& pBytes, const bool pBigEndian) -> std::uint64_t
                            { return Bytes::composeUInt64(pBytes, pBigEndian); },
           "Create a 64 bit unsigned integer from an eight byte sequence of a certain endianness.",
           py::arg("bytes"), py::arg("bigEndian") = true);

    pM.def("bitsetFromBytes", [](const std::vector<std::uint8_t>& pBytes, const std::size_t pBitSize) -> std::vector<bool>
                              {
                                  const boost::dynamic_bitset bitset = Bytes::bitsetFromBytes(pBytes, pBitSize);
                                  std::vector<bool> retVal(bitset.size(), false);
                                  for (std::size_t i = 0; i < bitset.size(); ++i)
                                      retVal[i] = bitset.test(bitset.size() - 1 - i);
                                  return retVal;
                              }, "Convert a sequence of bytes to a dynamic bitset.", py::arg("bytes"), py::arg("bitSize"));
    pM.def("bytesFromBitset", [](const std::vector<bool>& pBits, const std::size_t pByteSize) -> std::vector<std::uint8_t>
                              {
                                  boost::dynamic_bitset bitset(pBits.size());
                                  for (std::size_t i = 0; i < pBits.size(); ++i)
                                      bitset[i] = pBits[pBits.size() - 1 - i];
                                  return Bytes::bytesFromBitset(bitset, pByteSize);
                              }, "Convert a dynamic bitset to a sequence of bytes.", py::arg("bits"), py::arg("byteSize"));

    pM.def("byteVecFromStr", &Bytes::byteVecFromStr, "Interpret a character string as a sequence of bytes.", py::arg("str"));
    pM.def("strFromByteVec", &Bytes::strFromByteVec, "Interpret a sequence of bytes as a character string.", py::arg("vec"));

    pM.def("formatHex", [](const std::uint64_t pNumber) -> std::string { return Bytes::formatHex<std::uint64_t>(pNumber, false); },
           "Format an unsigned integer as hexadecimal literal.", py::arg("number"));
    pM.def("formatByteVec", &Bytes::formatByteVec,
           "Format a vector of 8 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.", py::arg("vec"));
    pM.def("formatUIntVec", &Bytes::formatUInt64Vec,
           "Format a vector of unsigned integers as brace-enclosed sequence of hexadecimal literals.", py::arg("vec"));
}
