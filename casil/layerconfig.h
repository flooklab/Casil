/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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

#ifndef CASIL_LAYERCONFIG_H
#define CASIL_LAYERCONFIG_H

#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

/*!
 * \brief Configuration object for layer components derived from LayerBase.
 *
 * This is basically a convenience wrapper around a Boost Property Tree that represents a parsed YAML document.
 * There are getters for different types of configuration values and you can easily check the tree structure and contained value types.
 */
class LayerConfig
{
public:
    LayerConfig() = default;                                            ///< Default constructor.
    explicit LayerConfig(const boost::property_tree::ptree& pTree);     ///< Constructor.
    LayerConfig(const LayerConfig&) = default;                          ///< Default copy constructor.
    LayerConfig(LayerConfig&&) = default;                               ///< Default move constructor.
    ~LayerConfig() = default;                                           ///< Default destructor.
    //
    LayerConfig& operator=(LayerConfig) = delete;                       ///< Deleted copy assignment operator.
    LayerConfig& operator=(LayerConfig&&) = delete;                     ///< Deleted move assignment operator.
    //
    bool operator==(const LayerConfig& pOther) const;                   ///< Equality operator.
    //
    bool contains(const LayerConfig& pOther, bool pCheckTypes = false) const;   ///< Check the configuration tree structure (and value types).
    //
    bool getBool(const std::string& pKey, bool pDefault = false) const;                     ///< Get a boolean configuration value.
    int getInt(const std::string& pKey, int pDefault = 0) const;                            ///< Get a (signed) integer configuration value.
    std::uint64_t getUInt(const std::string& pKey, std::uint64_t pDefault = 0x0u) const;    ///< Get an unsigned integer configuration value.
    double getDbl(const std::string& pKey, double pDefault = 0.0) const;                    ///< Get a floating point configuration value.
    std::string getStr(const std::string& pKey, const std::string& pDefault = "") const;    ///< Get a string-type configuration value.
    //
    std::vector<std::uint8_t> getByteSeq(const std::string& pKey, std::vector<std::uint8_t> pDefault = {}) const;
                                                                                            ///< \brief Get an 8 bit unsigned integer sequence
                                                                                            ///  from the configuration tree.
    std::vector<std::uint64_t> getUIntSeq(const std::string& pKey, std::vector<std::uint64_t> pDefault = {}) const;
                                                                                            ///< \brief Get a 64 bit unsigned integer sequence
                                                                                            ///  from the configuration tree.
    //
    boost::property_tree::ptree getRawTreeAt(const std::string& pPath) const;   ///< Get the raw configuration (sub-)tree at a specific path.
    //
    std::string toString() const;                                       ///< Format the configuration tree content as human-readable string.
    //
    static LayerConfig fromYAML(const std::string& pYAMLString);        ///< Create a configuration object from YAML format.

private:
    const boost::property_tree::ptree tree;                             ///< The configuration tree.
};

} // namespace casil

#endif // CASIL_LAYERCONFIG_H
