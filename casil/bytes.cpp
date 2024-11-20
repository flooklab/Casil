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

#include <casil/bytes.h>

#include <boost/dynamic_bitset.hpp>

#include <bitset>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <type_traits>

namespace
{

template<typename T>
    requires std::is_integral_v<T>
std::ostream& ostreamOperator(std::ostream& pOstream, const std::vector<T>& pVec)
{
    using casil::Bytes::formatHex;

    if (pVec.size() == 0)
        pOstream<<"{}";
    else if (pVec.size() == 1)
        pOstream<<"{"<<formatHex(pVec[0])<<"}";
    else
    {
        pOstream<<"{";
        for (std::size_t i = 0, sizeMin1 = pVec.size()-1; i < sizeMin1; ++i)
            pOstream<<formatHex(pVec[i])<<", ";
        pOstream<<formatHex(pVec.back())<<"}";
    }

    return pOstream;
}

template<typename T>
    requires std::is_integral_v<T>
std::string formatUIntVec(const std::vector<T>& pVec)
{
    std::ostringstream strm;
    ostreamOperator<T>(strm, pVec);
    return strm.str();
}

} // namespace

//

namespace casil::Bytes
{

boost::dynamic_bitset<> bitsetFromBytes(const std::vector<std::uint8_t>& pBytes, const std::size_t pBitSize)
{
    const std::size_t byteSize = pBytes.size();

    boost::dynamic_bitset bits(pBitSize);

    for (std::size_t i = 0; i < byteSize ; ++i)
    {
        const std::uint8_t currentByte = pBytes[byteSize-1-i];
        const std::bitset<8> currentBitset(currentByte);

        for (std::size_t j = 0; j < 8; ++j)
        {
            const std::size_t bitIdx = i*8 + j;

            if (bitIdx >= pBitSize)
                break;

            bits[bitIdx] = currentBitset[j];
        }
    }

    return bits;
}

std::vector<std::uint8_t> bytesFromBitset(const boost::dynamic_bitset<>& pBits, const std::size_t pByteSize)
{
    const std::size_t bitSize = pBits.size();

    std::vector<std::uint8_t> bytes(pByteSize, 0);

    for (std::size_t i = 0; i < pByteSize ; ++i)
    {
        std::bitset<8> currentBitset;

        for (std::size_t j = 0; j < 8; ++j)
        {
            const std::size_t bitIdx = i*8 + j;

            if (bitIdx >= bitSize)
                break;

            currentBitset[j] = pBits[bitIdx];
        }

        const std::uint8_t currentByte = currentBitset.to_ulong();

        bytes[pByteSize-1-i] = currentByte;
    }

    return bytes;
}

//

std::vector<std::uint8_t> byteVecFromStr(const std::string& pStr)
{
    return std::vector<std::uint8_t>(pStr.begin(), pStr.end());
}

std::string strFromByteVec(const std::vector<std::uint8_t>& pVec)
{
    return std::string(pVec.begin(), pVec.end());
}

void appendToByteVec(std::vector<std::uint8_t>& pVec, const std::vector<std::uint8_t>& pAppend)
{
    pVec.insert(pVec.end(), pAppend.begin(), pAppend.end());
}

//

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint8_t>& pByteVec)
{
    return ::ostreamOperator<std::uint8_t>(pOstream, pByteVec);
}

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint16_t>& pVec)
{
    return ::ostreamOperator<std::uint16_t>(pOstream, pVec);
}

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint32_t>& pVec)
{
    return ::ostreamOperator<std::uint32_t>(pOstream, pVec);
}

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint64_t>& pVec)
{
    return ::ostreamOperator<std::uint64_t>(pOstream, pVec);
}

std::string formatByteVec(const std::vector<std::uint8_t>& pVec)
{
    return ::formatUIntVec<std::uint8_t>(pVec);
}

std::string formatUInt16Vec(const std::vector<std::uint16_t>& pVec)
{
    return ::formatUIntVec<std::uint16_t>(pVec);
}

std::string formatUInt32Vec(const std::vector<std::uint32_t>& pVec)
{
    return ::formatUIntVec<std::uint32_t>(pVec);
}

std::string formatUInt64Vec(const std::vector<std::uint64_t>& pVec)
{
    return ::formatUIntVec<std::uint64_t>(pVec);
}

} // namespace casil::Bytes
