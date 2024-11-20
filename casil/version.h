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

namespace casil
{

namespace Version
{

enum class ReleaseType : std::uint8_t
{
    Alpha = 0,
    Beta = 1,
    ReleaseCandidate = 2,
    Normal = 3
};

inline constexpr int casilVersionMajor = 0;
inline constexpr int casilVersionMinor = 3;
inline constexpr int casilVersionPatch = 0;
inline constexpr ReleaseType casilVersionType = ReleaseType::Alpha;

std::string toString();

} // namespace Version

} // namespace casil

#endif // CASIL_VERSION_VERSION_H
