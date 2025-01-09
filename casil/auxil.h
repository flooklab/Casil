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

#ifndef CASIL_AUXIL_AUXIL_H
#define CASIL_AUXIL_AUXIL_H

#include <casil/asio.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace casil
{

/*!
 * \brief Auxiliary functions and classes.
 */
namespace Auxil
{

boost::property_tree::ptree propertyTreeFromYAML(const std::string& pYAMLString);   ///< Parse a YAML document into a Boost Property Tree.

//TODO this is now unused but maybe still useful in the future or for python; keep it?
std::vector<std::uint64_t> uintSeqFromYAML(const std::string& pYAMLString);         ///< Parse a sequence of unsigned integers from YAML format.

//

/*!
 * \brief Convert a floating point time duration to an std::chrono interval.
 *
 * Calculates rounded milliseconds from \p pSecs and returns them as chrono interval.
 *
 * \param pSecs Time duration in seconds.
 * \return Rounded millisecond interval.
 */
constexpr std::chrono::milliseconds getChronoMilliSecs(const double pSecs)
{
    return std::chrono::milliseconds(std::lround(pSecs*1e3));
}

/*!
 * \brief Convert a floating point time duration to an std::chrono interval.
 *
 * Calculates rounded microseconds from \p pSecs and returns them as chrono interval.
 *
 * \param pSecs Time duration in seconds.
 * \return Rounded microsecond interval.
 */
constexpr std::chrono::microseconds getChronoMicroSecs(const double pSecs)
{
    return std::chrono::microseconds(std::lround(pSecs*1e6));
}

//

/*!
 * \brief RAII wrapper to set and clear an atomic flag.
 *
 * Sets an atomic flag on construction and automatically clears it again on destruction.
 */
class AtomicFlagGuard
{
public:
    /*!
     * \brief Constructor.
     *
     * Sets the flag (via \c test_and_set() ).
     *
     * \param pFlag Atomic flag to be set/cleared.
     */
    explicit AtomicFlagGuard(std::atomic_flag& pFlag) :
        flag(pFlag)
    {
        flag.test_and_set();
    }
    /*!
     * \brief Destructor.
     *
     * Clears the flag.
     */
    ~AtomicFlagGuard()
    {
        flag.clear();
    }
private:
    std::atomic_flag& flag;     ///< The atomic flag to be set/cleared.
};

//

/*!
 * \brief RAII wrapper to run IO context threads for ASIO functionality.
 *
 * Starts ASIO processing threads on construction and automatically stops them again on destruction.
 *
 * \tparam numThreads Non-zero number of parallel threads to be used.
 */
template<unsigned int numThreads = 1>
class AsyncIORunner
{
    static_assert(numThreads != 0, "Number of threads must be non-zero.");

public:
    /*!
     * \brief Constructor.
     *
     * Starts the \p numThreads IO context threads.
     *
     * \throw std::runtime_error If the threads could not be started.
     */
    AsyncIORunner()
    {
        if (!ASIO::startRunIOContext(numThreads))
            throw std::runtime_error("Failed to start IO context threads.");
    }
    /*!
     * \brief Destructor.
     *
     * Stops the IO context threads.
     */
    ~AsyncIORunner()
    {
        ASIO::stopRunIOContext();
    }
};

} // namespace Auxil

} // namespace casil

#endif // CASIL_AUXIL_AUXIL_H
