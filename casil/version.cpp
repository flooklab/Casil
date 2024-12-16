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

#include <casil/version.h>

namespace casil::Version
{

/*!
 * \brief Get the library version formatted as string.
 *
 * The format is "MAJ.MIN.PATCH" plus "-alpha" / "-beta" / "-rc" for the pre-release types
 * ReleaseType::Alpha / ReleaseType::Beta / ReleaseType::ReleaseCandidate.
 *
 * \return The current library version.
 */
std::string toString()
{
    std::string verStr = std::to_string(casilVersionMajor) + "." + std::to_string(casilVersionMinor) + "." +
                         std::to_string(casilVersionPatch);

    if (casilVersionType == ReleaseType::Alpha)
        verStr += "-alpha";
    else if (casilVersionType == ReleaseType::Beta)
        verStr += "-beta";
    else if (casilVersionType == ReleaseType::ReleaseCandidate)
        verStr += "-rc";

    return verStr;
}

} // namespace casil::Version
