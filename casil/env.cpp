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

#include <casil/env.h>

#include <boost/predef/os/windows.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>

#include <cstdlib>

#define CASIL_XSTR(S) CASIL_STR(S)
#define CASIL_STR(S) #S

namespace
{

/*
 * Generates and returns a map that contains configured/determined value sets for every supported
 * environment variable. A value set is filled by reading set environment variables and then
 * adding compiled macro defaults; filesystem paths get split and added as individual elements.
 *
 * A value set will be empty if neither the variable is set at runtime nor a default is defined during compilation.
 */
std::map<std::string, std::set<std::string>, std::less<>> readEnv()
{
#if BOOST_OS_WINDOWS != 0
    static constexpr char const* separator = ";";
#else
    static constexpr char const* separator = ":";
#endif

    auto combinePaths = [](const std::string& pEnvPaths, const std::string& pMacroPaths) -> std::set<std::string>
    {
        std::vector<std::string> combinedPaths1;
        std::vector<std::string> combinedPaths2;

        boost::algorithm::split(combinedPaths1, pEnvPaths, boost::algorithm::is_any_of(separator), boost::algorithm::token_compress_on);
        boost::algorithm::split(combinedPaths2, pMacroPaths, boost::algorithm::is_any_of(separator), boost::algorithm::token_compress_on);

        std::set<std::string> combinedPaths;

        for (auto it : combinedPaths1)
            combinedPaths.insert(std::move(it));
        for (auto it : combinedPaths2)
            combinedPaths.insert(std::move(it));

        return combinedPaths;
    };

    std::map<std::string, std::set<std::string>, std::less<>> env;

    //SCPI device descriptions

    std::string devDescsMacro;
#ifdef CASIL_DEV_DESC_DIRS
    devDescsMacro = CASIL_XSTR(CASIL_DEV_DESC_DIRS);
#endif

    const char *const getenvPtr = std::getenv("CASIL_DEV_DESC_DIRS");
    std::string devDescsEnv = (getenvPtr ? getenvPtr : "");

    env.insert({"CASIL_DEV_DESC_DIRS", combinePaths(devDescsEnv, devDescsMacro)});

    return env;
}

} // namespace

namespace casil::Env
{

/*!
 * \brief Get a map of all Casil environment variables.
 *
 * Returns a map that contains configured/determined value sets for every supported environment variable.
 * A value set is filled by reading set environment variables and then adding compiled macro defaults;
 * filesystem paths get split and added as individual elements. See Env for more details and a list of supported variables.
 *
 * The value sets might be empty if neither the variable is set at runtime nor a default is defined during compilation.
 *
 * Use getEnv(std::string_view) to get just for a single variable.
 *
 * \return Map of variable value sets for all supported environment variables.
 */
const std::map<std::string, std::set<std::string>, std::less<>>& getEnv()
{
    const static std::map<std::string, std::set<std::string>, std::less<>> env = ::readEnv();

    return env;
}

/*!
 * \brief Get a specific Casil environment variable.
 *
 * Gets a single entry of the getEnv() map.
 *
 * \throws std::invalid_argument If \p pVarName is not supported (see Env for a list of supported variables).
 *
 * \param pVarName The name of the environment variable.
 * \return A set of all determined values for \p pVarName.
 */
const std::set<std::string>& getEnv(const std::string_view pVarName)
{
    const std::map<std::string, std::set<std::string>, std::less<>>& env = getEnv();

    const auto it = env.find(pVarName);

    if (it == env.end())
        throw std::invalid_argument("The environment variable \"" + std::string(pVarName) + "\" is not available through this function.");

    return it->second;
}

} // namespace casil::Env
