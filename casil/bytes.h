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
 * Contains conversion functions between unsigned integers and their bit and byte sequence representations,
 * string and stream formatting for unsigned integers and similar things.
 */
namespace Bytes
{

/*!
 * \brief Check if type is a fixed width (8/16/32/64 bits) unsigned integer.
 *
 * \p T must be equivalent to one of the \c std::uintX_t types.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept IsUnsignedIntNType = (std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t> ||
                              std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>);

template<typename... Ts>
    requires (IsUnsignedIntNType<Ts> && ...)
constexpr std::vector<std::uint8_t> composeByteVec(bool pBigEndian, Ts... pArgs);   ///< \brief Compose a byte sequence with a certain
                                                                                    ///  endianness from a number of unsigned integers.

constexpr std::uint16_t composeUInt16(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 16 bit unsigned integer from
                                                                                    ///  a two byte sequence of a certain endianness.
constexpr std::uint16_t composeUInt16(std::span<const std::uint8_t, 2> pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 16 bit unsigned integer from
                                                                                    ///  a two byte sequence of a certain endianness.
constexpr std::uint32_t composeUInt32(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 32 bit unsigned integer from
                                                                                    ///  a four byte sequence of a certain endianness.
constexpr std::uint32_t composeUInt32(std::span<const std::uint8_t, 4> pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 32 bit unsigned integer from
                                                                                    ///  a four byte sequence of a certain endianness.
constexpr std::uint64_t composeUInt64(const std::vector<std::uint8_t>& pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 64 bit unsigned integer from
                                                                                    ///  an eight byte sequence of a certain endianness.
constexpr std::uint64_t composeUInt64(std::span<const std::uint8_t, 8> pBytes, bool pBigEndian = true);
                                                                                    ///< \brief Create a 64 bit unsigned integer from
                                                                                    ///  an eight byte sequence of a certain endianness.

//

boost::dynamic_bitset<> bitsetFromBytes(const std::vector<std::uint8_t>& pBytes, std::size_t pBitSize);
                                                                                    ///< Convert a sequence of bytes to a dynamic bitset.
std::vector<std::uint8_t> bytesFromBitset(const boost::dynamic_bitset<>& pBits, std::size_t pByteSize);
                                                                                    ///< Convert a dynamic bitset to a sequence of bytes.

//

std::vector<std::uint8_t> byteVecFromStr(const std::string& pStr);                  ///< Interpret a character string as a sequence of bytes.
std::string strFromByteVec(const std::vector<std::uint8_t>& pVec);                  ///< Interpret a sequence of bytes as a character string.

void appendToByteVec(std::vector<std::uint8_t>& pVec, const std::vector<std::uint8_t>& pAppend);    ///< Concatenate two sequences of bytes.

//

template<typename T>
    requires IsUnsignedIntNType<T>
std::string formatHex(T pUnsignedNumber, bool pAddTypePadding = false);             ///< Format an unsigned integer as hexadecimal literal.

std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint8_t>& pByteVec);
                                                                                    ///< \brief Output a vector of 8 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint16_t>& pVec);
                                                                                    ///< \brief Output a vector of 16 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint32_t>& pVec);
                                                                                    ///< \brief Output a vector of 32 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint64_t>& pVec);
                                                                                    ///< \brief Output a vector of 64 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.

std::string formatByteVec(const std::vector<std::uint8_t>& pVec);                   ///< \brief Format a vector of 8 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::string formatUInt16Vec(const std::vector<std::uint16_t>& pVec);                ///< \brief Format a vector of 16 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::string formatUInt32Vec(const std::vector<std::uint32_t>& pVec);                ///< \brief Format a vector of 32 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.
std::string formatUInt64Vec(const std::vector<std::uint64_t>& pVec);                ///< \brief Format a vector of 64 bit unsigned integers
                                                                                    ///  as brace-enclosed sequence of hexadecimal literals.

//Template and constexpr function definitions


/*!
 * \brief Compose a byte sequence with a certain endianness from a number of unsigned integers.
 *
 * The sequence is composed from all \p pArgs... in passed order and each \p pArgs is treated according to the endianness \p pBigEndian.
 *
 * That means for \p pArgs... = <tt>{val_1, ..., val_m}</tt>, \p Ts... = <tt>{T_1, ..., T_m}</tt>,
 * \p MSB_i = <tt>mostSignificantByte(T_i)</tt>, \p LSB_i = <tt>leastSignificantByte(T_i)</tt> the returned sequence is:
 * - <tt>{val_1[MSB_1], ..., val_1[LSB_1], ..., val_m[MSB_m], ..., val_m[LSB_m]}</tt>, for \p pBigEndian = true
 * - <tt>{val_1[LSB_1], ..., val_1[MSB_1], ..., val_m[LSB_m], ..., val_m[MSB_m]}</tt>, for \p pBigEndian = false
 *
 * \tparam Ts Types of passed unsigned integers \p pArgs.
 * \param pBigEndian Use big endian byte order for each number if true and little endian else.
 * \param pArgs Unsigned integers to add to the returned sequence in the passed order.
 * \return The composed byte sequence.
 */
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

/*!
 * \brief Implementation details for Bytes.
 */
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

/*!
 * \brief Create a 16 bit unsigned integer from a two byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], pBytes[1]}</tt> --> <tt>{val[MSB], val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[1], pBytes[0]}</tt> --> <tt>{val[MSB], val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \throws std::invalid_argument If \p pBytes has a size other than 2.
 *
 * \param pBytes The two byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint16_t composeUInt16(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 2)
        throw std::invalid_argument("Number of bytes must be 2.");
    return BytesImpl::composeUInt<std::uint16_t, 2>(pBigEndian, std::span<const std::uint8_t, 2>(pBytes.begin(), 2));
}

/*!
 * \brief Create a 16 bit unsigned integer from a two byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], pBytes[1]}</tt> --> <tt>{val[MSB], val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[1], pBytes[0]}</tt> --> <tt>{val[MSB], val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \param pBytes The two byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint16_t composeUInt16(const std::span<const std::uint8_t, 2> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint16_t, 2>(pBigEndian, pBytes);
}

/*!
 * \brief Create a 32 bit unsigned integer from a four byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], ..., pBytes[3]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[3], ..., pBytes[0]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \throws std::invalid_argument If \p pBytes has a size other than 4.
 *
 * \param pBytes The four byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint32_t composeUInt32(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 4)
        throw std::invalid_argument("Number of bytes must be 4.");
    return BytesImpl::composeUInt<std::uint32_t, 4>(pBigEndian, std::span<const std::uint8_t, 4>(pBytes.begin(), 4));
}

/*!
 * \brief Create a 32 bit unsigned integer from a four byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], ..., pBytes[3]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[3], ..., pBytes[0]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \param pBytes The four byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint32_t composeUInt32(const std::span<const std::uint8_t, 4> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint32_t, 4>(pBigEndian, pBytes);
}

/*!
 * \brief Create a 64 bit unsigned integer from an eight byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], ..., pBytes[7]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[7], ..., pBytes[0]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \throws std::invalid_argument If \p pBytes has a size other than 8.
 *
 * \param pBytes The eight byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint64_t composeUInt64(const std::vector<std::uint8_t>& pBytes, const bool pBigEndian)
{
    if (pBytes.size() != 8)
        throw std::invalid_argument("Number of bytes must be 8.");
    return BytesImpl::composeUInt<std::uint64_t, 8>(pBigEndian, std::span<const std::uint8_t, 8>(pBytes.begin(), 8));
}

/*!
 * \brief Create a 64 bit unsigned integer from an eight byte sequence of a certain endianness.
 *
 * The sequence \p pBytes is interpreted as an unsigned integer number \p val as follows:
 * - <tt>{pBytes[0], ..., pBytes[7]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = true
 * - <tt>{pBytes[7], ..., pBytes[0]}</tt> --> <tt>{val[MSB], ..., val[LSB]}</tt>, for \p pBigEndian = false
 *
 * Here \p MSB denotes the most significant byte of \p val and \p LSB its least significant byte.
 *
 * \param pBytes The eight byte long sequence representing the value to be returned.
 * \param pBigEndian Assume big endian byte order for \p pBytes if true and little endian order else.
 * \return The interpreted unsigned integer number.
 */
constexpr std::uint64_t composeUInt64(const std::span<const std::uint8_t, 8> pBytes, const bool pBigEndian)
{
    return BytesImpl::composeUInt<std::uint64_t, 8>(pBigEndian, pBytes);
}

//

/*!
 * \brief Format an unsigned integer as hexadecimal literal.
 *
 * Returns a string showing \p pUnsignedNumber as hexadecimal literal (like "0xABC").
 *
 * If \p pAddTypePadding is true, a zero-padding is added according to the byte length of \p T.
 *
 * \tparam T Type of the unsigned integer.
 * \param pUnsignedNumber The number to format.
 * \param pAddTypePadding Add integer size-dependent padding zeros after "0x" prefix if true.
 * \return The formatted string.
 */
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
