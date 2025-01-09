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
 * Provides access to environment variables used for configuration of Casil behavior (e.g. special paths, ...),
 * combined with possible defaults of them that can be compiled into the library via macro definitions.
 *
 * For each variable a set of values is defined, in which environment variables (if set) come first and macro defaults last.
 *
 * In case of variables used for file/directory paths, individual paths will be automatically
 * split at OS-specific separators (':' / ';') and added as individual set elements.
 *
 * See getEnv() for details on how to access specific variables.
 *
 * List of supported variables:
 * - \c CASIL_DEV_DESC_DIRS: Directories containing \ref Layers::HL::SCPI "SCPI" device description files.
 */
namespace Env
{

const std::map<std::string, std::set<std::string>, std::less<>>& getEnv();  ///< Get a map of all Casil environment variables.

const std::set<std::string>& getEnv(std::string_view pVarName);             ///< Get a specific Casil environment variable.

} // namespace Env

} // namespace casil

#endif // CASIL_ENV_ENV_H
