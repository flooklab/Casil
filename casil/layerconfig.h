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

#ifndef CASIL_LAYERCONFIG_H
#define CASIL_LAYERCONFIG_H

#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

class LayerConfig
{
public:
    LayerConfig() = default;
    explicit LayerConfig(const boost::property_tree::ptree& pTree);
    LayerConfig(const LayerConfig&) = default;
    LayerConfig(LayerConfig&&) = default;
    ~LayerConfig() = default;
    //
    LayerConfig& operator=(LayerConfig) = delete;
    LayerConfig& operator=(LayerConfig&&) = delete;
    //
    bool operator==(const LayerConfig& pOther) const;
    //
    bool contains(const LayerConfig& pOther, bool pCheckTypes = false) const;
    //
    bool getBool(const std::string& pKey, bool pDefault = false) const;
    int getInt(const std::string& pKey, int pDefault = 0) const;
    std::uint64_t getUInt(const std::string& pKey, std::uint64_t pDefault = 0x0u) const;
    double getDbl(const std::string& pKey, double pDefault = 0.0) const;
    std::string getStr(const std::string& pKey, std::string pDefault = "") const;
    //
    std::vector<std::uint8_t> getByteSeq(const std::string& pKey, std::vector<std::uint8_t> pDefault = {}) const;
    std::vector<std::uint64_t> getUIntSeq(const std::string& pKey, std::vector<std::uint64_t> pDefault = {}) const;
    //
    static LayerConfig fromYAML(const std::string& pYAMLString);

private:
    const boost::property_tree::ptree tree;
};

} // namespace casil

#endif // CASIL_LAYERCONFIG_H
