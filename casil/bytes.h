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

#ifndef CASIL_BYTES_BYTES_H
#define CASIL_BYTES_BYTES_H

#include <boost/dynamic_bitset_fwd.hpp>

#include <cstdint>
#include <format>
#include <iosfwd>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace casil
{

/*!
 * \brief Auxiliary functions for bit/byte manipulation.
 *
 * \todo Detailed doc
 */
namespace Bytes
{

template<typename T>
concept IsUnsignedIntNType = (std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
                              std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>);

template<typename... Ts>
    requires (IsUnsignedIntNType<Ts> && ...)
constexpr std::vector<std::uint8_t> composeByteVec(bool pBigEndian, Ts... pArgs);

constexpr std::uint16_t composeUInt16(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
constexpr std::uint16_t composeUInt16(std::span<const std::uint8_t, 2> pBytes, bool pBigEndian = true);
constexpr std::uint32_t composeUInt32(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
constexpr std::uint32_t composeUInt32(std::span<const std::uint8_t, 4> pBytes, bool pBigEndian = true);
constexpr std::uint64_t composeUInt64(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
constexpr std::uint64_t composeUInt64(std::span<const std::uint8_t, 8> pBytes, bool pBigEndian = true);

//

boost::dynamic_bitset<> bitsetFromBytes(const std::vector<std::uint8_t>& pBytes, std::size_t pBitSize);
std::vector<std::uint8_t> bytesFromBitset(const boost::dynamic_bitset<>& pBits, std::size_t pByteSize);

//

std::vector<std::uint8_t> byteVecFromStr(const std::string& pStr);
std::string strFromByteVec(const std::vector<std::uint8_t>& pVec);

void appendToByteVec(std::vector<std::uint8_t>& pVec, const std::vector<std::uint8_t>& pAppend);

//

template<typename T>
    requires IsUnsignedIntNType<T>
std::string formatHex(T pUnsignedNumber, bool pAddTypePadding = false);

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint8_t>& pByteVec);
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint16_t>& pVec);
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint32_t>& pVec);
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint64_t>& pVec);

std::string formatByteVec(const std::vector<std::uint8_t>& pVec);

std::string formatUInt16Vec(const std::vector<std::uint16_t>& pVec);
std::string formatUInt32Vec(const std::vector<std::uint32_t>& pVec);
std::string formatUInt64Vec(const std::vector<std::uint64_t>& pVec);


//Template and constexpr function definitions


template<typename... Ts>
    requires (IsUnsignedIntNType<Ts> && ...)
constexpr std::vector<std::uint8_t> composeByteVec(const bool pBigEndian, Ts... pArgs)
{
    std::vector<std::uint8_t> retVal;

    retVal.reserve((sizeof(Ts) + ...));

    ([pBigEndian, pArgs, &retVal]()
    {
        if constexpr (std::is_same_v<Ts, std::uint8_t>)
        {
            retVal.push_back(pArgs);
        }
        else if constexpr (std::is_same_v<Ts, std::uint16_t>)
        {
            if (pBigEndian)
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF00u) >> 8));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FFu)));
            }
            else
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FFu)));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF00u) >> 8));
            }
        }
        else if constexpr (std::is_same_v<Ts, std::uint32_t>)
        {
            if (pBigEndian)
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF000000u) >> 24));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FF0000u) >> 16));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000FF00u) >> 8));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000FFu)));
            }
            else
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000FFu)));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000FF00u) >> 8));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FF0000u) >> 16));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF000000u) >> 24));
            }
        }
        else if constexpr (std::is_same_v<Ts, std::uint64_t>)
        {
            if (pBigEndian)
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF00000000000000u) >> 56));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FF000000000000u) >> 48));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000FF0000000000u) >> 40));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000FF00000000u) >> 32));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00000000FF000000u) >> 24));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000000000FF0000u) >> 16));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000000000FF00u) >> 8));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00000000000000FFu)));
            }
            else
            {
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00000000000000FFu)));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000000000FF00u) >> 8));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000000000FF0000u) >> 16));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00000000FF000000u) >> 24));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x000000FF00000000u) >> 32));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x0000FF0000000000u) >> 40));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0x00FF000000000000u) >> 48));
                retVal.push_back(static_cast<std::uint8_t>((pArgs & 0xFF00000000000000u) >> 56));
            }
        }
        else
            static_assert(false, "This cannot happen.");
    }(), ...);

    return retVal;
}

namespace BytesImpl
{

template<typename T, std::size_t N>
concept TypeMatchesByteLength = (std::is_same_v<std::integral_constant<std::size_t, 2>, std::integral_constant<std::size_t, N>> &&
                                 std::is_same_v<T, std::uint16_t>) ||
                                (std::is_same_v<std::integral_constant<std::size_t, 4>, std::integral_constant<std::size_t, N>> &&
                                 std::is_same_v<T, std::uint32_t>) ||
                                (std::is_same_v<std::integral_constant<std::size_t, 8>, std::integral_constant<std::size_t, N>> &&
                                 std::is_same_v<T, std::uint64_t>);

template<typename T, std::size_t N>
    requires (IsUnsignedIntNType<T> && !std::is_same_v<T, std::uint8_t> && TypeMatchesByteLength<T, N>)
constexpr T composeUInt(const bool pBigEndian, const std::span<const std::uint8_t, N> pBytes)
{
    if constexpr (N == 2)
    {
        if (pBigEndian)
            return ((static_cast<std::uint16_t>(pBytes[0]) <<  8) | (static_cast<std::uint16_t>(pBytes[1]) <<  0));
        else
            return ((static_cast<std::uint16_t>(pBytes[1]) <<  8) | (static_cast<std::uint16_t>(pBytes[0]) <<  0));
    }
    else if constexpr (N == 4)
    {
        if (pBigEndian)
        {
            return ((static_cast<std::uint32_t>(pBytes[0]) << 24) | (static_cast<std::uint32_t>(pBytes[1]) << 16) |
                    (static_cast<std::uint32_t>(pBytes[2]) <<  8) | (static_cast<std::uint32_t>(pBytes[3]) <<  0));
        }
        else
        {
            return ((static_cast<std::uint32_t>(pBytes[3]) << 24) | (static_cast<std::uint32_t>(pBytes[2]) << 16) |
                    (static_cast<std::uint32_t>(pBytes[1]) <<  8) | (static_cast<std::uint32_t>(pBytes[0]) <<  0));
        }
    }
    else if constexpr (N == 8)
    {
        if (pBigEndian)
        {
            return ((static_cast<std::uint64_t>(pBytes[0]) << 56) | (static_cast<std::uint64_t>(pBytes[1]) << 48) |
                    (static_cast<std::uint64_t>(pBytes[2]) << 40) | (static_cast<std::uint64_t>(pBytes[3]) << 32) |
                    (static_cast<std::uint64_t>(pBytes[4]) << 24) | (static_cast<std::uint64_t>(pBytes[5]) << 16) |
                    (static_cast<std::uint64_t>(pBytes[6]) <<  8) | (static_cast<std::uint64_t>(pBytes[7]) <<  0));
        }
        else
        {
            return ((static_cast<std::uint64_t>(pBytes[7]) << 56) | (static_cast<std::uint64_t>(pBytes[6]) << 48) |
                    (static_cast<std::uint64_t>(pBytes[5]) << 40) | (static_cast<std::uint64_t>(pBytes[4]) << 32) |
                    (static_cast<std::uint64_t>(pBytes[3]) << 24) | (static_cast<std::uint64_t>(pBytes[2]) << 16) |
                    (static_cast<std::uint64_t>(pBytes[1]) <<  8) | (static_cast<std::uint64_t>(pBytes[0]) <<  0));
        }
    }
    else
        static_assert(false, "This cannot happen.");
}

} // namespace BytesImpl

constexpr std::uint16_t composeUInt16(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 2)
        throw std::invalid_argument("Number of bytes must be 2.");
    return BytesImpl::composeUInt<std::uint16_t, 2>(pBigEndian, std::span<const std::uint8_t, 2>(pBytes.begin(), 2));
}

constexpr std::uint16_t composeUInt16(const std::span<const std::uint8_t, 2> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint16_t, 2>(pBigEndian, pBytes);
}

constexpr std::uint32_t composeUInt32(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 4)
        throw std::invalid_argument("Number of bytes must be 4.");
    return BytesImpl::composeUInt<std::uint32_t, 4>(pBigEndian, std::span<const std::uint8_t, 4>(pBytes.begin(), 4));
}

constexpr std::uint32_t composeUInt32(const std::span<const std::uint8_t, 4> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint32_t, 4>(pBigEndian, pBytes);
}

constexpr std::uint64_t composeUInt64(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 8)
        throw std::invalid_argument("Number of bytes must be 8.");
    return BytesImpl::composeUInt<std::uint64_t, 8>(pBigEndian, std::span<const std::uint8_t, 8>(pBytes.begin(), 8));
}

constexpr std::uint64_t composeUInt64(const std::span<const std::uint8_t, 8> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint64_t, 8>(pBigEndian, pBytes);
}

//

template<typename T>
    requires IsUnsignedIntNType<T>
std::string formatHex(const T pUnsignedNumber, const bool pAddTypePadding)
{
    if (pAddTypePadding)
    {
        if constexpr (std::is_same_v<T, std::uint8_t>)
            return std::format("0x{:02X}", pUnsignedNumber);
        else if constexpr (std::is_same_v<T, std::uint16_t>)
            return std::format("0x{:04X}", pUnsignedNumber);
        else if constexpr (std::is_same_v<T, std::uint32_t>)
            return std::format("0x{:08X}", pUnsignedNumber);
        else if constexpr (std::is_same_v<T, std::uint64_t>)
            return std::format("0x{:016X}", pUnsignedNumber);
        else
            static_assert("Invalid type.");
    }
    else
        return std::format("0x{:X}", pUnsignedNumber);
}

} // namespace Bytes

} // namespace casil

#endif // CASIL_BYTES_BYTES_H
