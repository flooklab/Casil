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

#ifndef CASIL_VERSION_VERSION_H
#define CASIL_VERSION_VERSION_H

#include <cstdint>
#include <string>

/*!
 * \brief Casil base namespace.
 */
namespace casil
{

/*!
 * \brief Library version information.
 *
 * The current version of the Casil library is defined here by \ref casilVersionMajor, \ref casilVersionMinor,
 * \ref casilVersionPatch and \ref casilVersionType.
 */
namespace Version
{

/*!
 * \brief Release type of the released library version.
 *
 * Before a stable release of the library, a release can have one of the listed pre-release states to specify the current development status.
 * Use toString() to obtain a the current version as a combined string.
 */
enum class ReleaseType : std::uint8_t
{
    Alpha = 0,              ///< Pre-release \e alpha status.
    Beta = 1,               ///< Pre-release \e beta status.
    ReleaseCandidate = 2,   ///< Pre-release \e release \e candidate status.
    Normal = 3              ///< %Normal/final release.
};

inline constexpr int casilVersionMajor = 0;                         ///< Library version major number.
inline constexpr int casilVersionMinor = 4;                         ///< Library version minor number.
inline constexpr int casilVersionPatch = 0;                         ///< Library version patch number.
inline constexpr ReleaseType casilVersionType = ReleaseType::Alpha; ///< Library version release type.

std::string toString();                                             ///< Get the library version formatted as string.

} // namespace Version

} // namespace casil

#endif // CASIL_VERSION_VERSION_H
