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

#include <pycasil/pycasil_utils.h>

#include <boost/dynamic_bitset.hpp>

#include <cstddef>

namespace PyCasilUtils
{

/*
 * Convert a vector of bools to a dynamic bitset.
 *
 * Creates and returns a bitset with the same size as 'pBits' and reflecting the content of 'pBits'.
 * The first element in 'pBits' ends up as the most significant bit in the returned bitset:
 * bitset[(size-1):0] = pBits[0:(size-1)]
 */
boost::dynamic_bitset<> bitsetFromBoolVec(const std::vector<bool>& pBits)
{
    boost::dynamic_bitset bitset(pBits.size());
    for (std::size_t i = 0; i < pBits.size(); ++i)
        bitset[i] = pBits[pBits.size() - 1 - i];
    return bitset;
}

/*
 * Convert a dynamic bitset to a vector of bools.
 *
 * Creates and returns a vector with the same size as 'pBits' and reflecting the content of 'pBits'.
 * The most significant bit in 'pBits' ends up as the first element in the returned vector:
 * vector[0:(size-1)] = pBits[(size-1):0]
 */
std::vector<bool> boolVecFromBitset(const boost::dynamic_bitset<>& pBits)
{
    std::vector<bool> boolVec(pBits.size(), false);
    for (std::size_t i = 0; i < pBits.size(); ++i)
        boolVec[i] = pBits.test(pBits.size() - 1 - i);
    return boolVec;
}

} // namespace PyCasilUtils
