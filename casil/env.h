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

#ifndef CASIL_ENV_ENV_H
#define CASIL_ENV_ENV_H

#include <functional>
#include <string>
#include <string_view>
#include <map>
#include <set>

namespace casil
{

/*!
 * \brief Handling of environment variables needed by the library.
 *
 * \todo Detailed doc
 */
namespace Env
{

const std::map<std::string, std::set<std::string>, std::less<>>& getEnv();

const std::set<std::string>& getEnv(std::string_view pVarName);

} // namespace Env

} // namespace casil

#endif // CASIL_ENV_ENV_H
