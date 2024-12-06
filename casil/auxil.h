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

namespace Auxil
{

boost::property_tree::ptree propertyTreeFromYAML(const std::string& pYAMLString);

//TODO this is now unused but maybe still useful in the future or for python; keep it?
std::vector<std::uint64_t> uintSeqFromYAML(const std::string& pYAMLString);

//

constexpr std::chrono::milliseconds getChronoMilliSecs(const double pSecs)
{
    return std::chrono::milliseconds(std::lround(pSecs*1e3));
}

constexpr std::chrono::microseconds getChronoMicroSecs(const double pSecs)
{
    return std::chrono::microseconds(std::lround(pSecs*1e6));
}

//

class AtomicFlagGuard
{
public:
    explicit AtomicFlagGuard(std::atomic_flag& pFlag) :
        flag(pFlag)
    {
        flag.test_and_set();
    }
    ~AtomicFlagGuard()
    {
        flag.clear();
    }
private:
    std::atomic_flag& flag;
};

//

template<unsigned int numThreads = 1>
struct AsyncIORunner
{
    static_assert(numThreads != 0, "Number of threads must be non-zero.");
    AsyncIORunner()
    {
        if (!ASIO::startRunIOContext(numThreads))
            throw std::runtime_error("Failed to start IO context threads.");
    }
    ~AsyncIORunner()
    {
        ASIO::stopRunIOContext();
    }
};

} // namespace Auxil

} // namespace casil

#endif // CASIL_AUXIL_AUXIL_H