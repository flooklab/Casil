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

#include <casil/bytes.h>

#include <boost/dynamic_bitset.hpp>

#include <bitset>
#include <cstddef>
#include <ostream>
#include <sstream>
#include <type_traits>

namespace
{

/*
 * Outputs 'pVec' to 'pOstream' as "{S1, S2, ..., SN}" with each Si being an integer of
 * type 'T' as formatted by formatHex() without zero-padding and where Si = pVec[i-1].
 */
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

/*
 * Returns 'pVec' as string formatted using ostreamOperator<T>().
 */
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

/*!
 * \brief Convert a sequence of bytes to a dynamic bitset.
 *
 * Assumes that \p pBytes represents a \p pBitSize long bit sequence (an integer number) in big endian byte order
 * (i.e. \p pBytes[0] is the most significant byte) and returns these \p pBitSize bits as a dynamic bitset.
 *
 * \note \p pBitSize should of course not be larger than eight times the size of \p pBytes; this is not checked here.
 *
 * \param pBytes The byte sequence to convert.
 * \param pBitSize Number of bits to convert from \p pBytes.
 * \return The \p pBitSize bits.
 */
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

/*!
 * \brief Convert a dynamic bitset to a sequence of bytes.
 *
 * Assumes that \p pBits (as an integer number) can be represented by a \p pByteSize long byte sequence \p seq and returns it.
 * \p seq will be in big endian byte order (i.e. \p pBits[0] ends up in \p seq[pByteSize-1], which is the least significant byte).
 *
 * \note 8 * \p pByteSize should of course not be smaller than the size of \p pBits; this is not checked here.
 *
 * \param pBits The bit sequence to convert.
 * \param pByteSize Number of bytes to convert from \p pBits.
 * \return The \p pByteSize bytes.
 */
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

/*!
 * \brief Interpret a character string as a sequence of bytes.
 *
 * Takes each character of \p pStr and adds it to the returned byte sequence
 * as the 8 bit unsigned integer representation of its ASCII code.
 *
 * \param pStr The character string to be converted.
 * \return The equivalent byte sequence.
 */
std::vector<std::uint8_t> byteVecFromStr(const std::string& pStr)
{
    return std::vector<std::uint8_t>(pStr.begin(), pStr.end());
}

/*!
 * \brief Interpret a sequence of bytes as a character string.
 *
 * Takes each byte of \p pVec as 8 bit unsigned integer and adds a character
 * with ASCII code equal to this integer to the returned string.
 *
 * \param pVec The byte sequence to be converted.
 * \return The equivalent character string.
 */
std::string strFromByteVec(const std::vector<std::uint8_t>& pVec)
{
    return std::string(pVec.begin(), pVec.end());
}

/*!
 * \brief Concatenate two sequences of bytes.
 *
 * \param pVec Byte sequence to be appended to.
 * \param pAppend Byte sequence to append to \p pVec.
 */
void appendToByteVec(std::vector<std::uint8_t>& pVec, const std::vector<std::uint8_t>& pAppend)
{
    pVec.insert(pVec.end(), pAppend.begin(), pAppend.end());
}

//

/*!
 * \brief Output a vector of 8 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * Outputs \p pVec to \p pOstream as <tt>"{S1, S2, ..., SN}"</tt> with each \c Si being
 * formatted by formatHex() without zero-padding and where <tt>Si = pVec[i-1]</tt>.
 *
 * \param pOstream The stream to output to.
 * \param pVec The vector to be formatted/outputted.
 * \return \p pOstream.
 */
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint8_t>& pVec)
{
    return ::ostreamOperator<std::uint8_t>(pOstream, pVec);
}

/*!
 * \brief Output a vector of 16 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails operator<<(std::ostream&, const std::vector<std::uint8_t>&)
 */
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint16_t>& pVec)
{
    return ::ostreamOperator<std::uint16_t>(pOstream, pVec);
}

/*!
 * \brief Output a vector of 32 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails operator<<(std::ostream&, const std::vector<std::uint8_t>&)
 */
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint32_t>& pVec)
{
    return ::ostreamOperator<std::uint32_t>(pOstream, pVec);
}

/*!
 * \brief Output a vector of 64 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails operator<<(std::ostream&, const std::vector<std::uint8_t>&)
 */
std::ostream& operator<<(std::ostream& pOstream, const std::vector<std::uint64_t>& pVec)
{
    return ::ostreamOperator<std::uint64_t>(pOstream, pVec);
}

/*!
 * \brief Format a vector of 8 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * Returns a string showing \p pVec as <tt>"{S1, S2, ..., SN}"</tt> with each \c Si being
 * formatted by formatHex() without zero-padding and where <tt>Si = pVec[i-1]</tt>.
 *
 * \param pVec The vector to be formatted.
 * \return The formatted string.
 */
std::string formatByteVec(const std::vector<std::uint8_t>& pVec)
{
    return ::formatUIntVec<std::uint8_t>(pVec);
}

/*!
 * \brief Format a vector of 16 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails formatByteVec()
 */
std::string formatUInt16Vec(const std::vector<std::uint16_t>& pVec)
{
    return ::formatUIntVec<std::uint16_t>(pVec);
}

/*!
 * \brief Format a vector of 32 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails formatByteVec()
 */
std::string formatUInt32Vec(const std::vector<std::uint32_t>& pVec)
{
    return ::formatUIntVec<std::uint32_t>(pVec);
}

/*!
 * \brief Format a vector of 64 bit unsigned integers as brace-enclosed sequence of hexadecimal literals.
 *
 * \copydetails formatByteVec()
 */
std::string formatUInt64Vec(const std::vector<std::uint64_t>& pVec)
{
    return ::formatUIntVec<std::uint64_t>(pVec);
}

} // namespace casil::Bytes
