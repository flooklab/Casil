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

#include <casil/auxil.h>

#include <casil/layerconfig.h>

#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/impl.h>
#include <yaml-cpp/node/iterator.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/node/type.h>

#include <boost/property_tree/ptree.hpp>

#include <cstddef>
#include <functional>

namespace casil::Auxil
{

/*!
 * \brief Parse a YAML document into a Boost Property Tree.
 *
 * - The top node of the YAML document must be a sequence or map.
 * - Elements of sequences and maps are iteratively added as sub-trees to the current branch of the tree.
 * - In case of a sequence, these sub-trees are assigned to made-up keys "#0", "#1", etc...
 * - Scalar elements are represented as a sub-tree with a value (see \c boost::property_tree::ptree::data() ) and no childs.
 * - Elements of sequence/map type are recursively parsed sub-trees.
 *
 * \param pYAMLString The YAML document to be parsed.
 * \return Property Tree representing the YAML document structure/content.
 */
boost::property_tree::ptree propertyTreeFromYAML(const std::string& pYAMLString)
{
    using boost::property_tree::ptree;

    std::function<void(ptree&, const YAML::Node&)> fillTree = [&fillTree](ptree& pTree, const YAML::Node& pNode) -> void
    {
        if (pNode.Type() != YAML::NodeType::Sequence && pNode.Type() != YAML::NodeType::Map)
            return;

        std::size_t tSequenceCtr = 0;

        for (const auto& it : pNode)
        {
            const YAML::Node& childNode = ((pNode.Type() == YAML::NodeType::Sequence) ? it : it.second);
            const std::string childKey = ((pNode.Type() == YAML::NodeType::Sequence) ? ("#" + std::to_string(tSequenceCtr++))
                                                                                     : it.first.as<std::string>());

            ptree tSubTree;

            switch (childNode.Type())
            {
                case YAML::NodeType::Scalar:
                {
                    tSubTree.put_value(childNode.as<std::string>());
                    break;
                }
                case YAML::NodeType::Sequence:
                {
                    fillTree(tSubTree, childNode);
                    break;
                }
                case YAML::NodeType::Map:
                {
                    fillTree(tSubTree, childNode);
                    break;
                }
                case YAML::NodeType::Null:
                case YAML::NodeType::Undefined:
                default:
                    break;
            }

            pTree.add_child(childKey, tSubTree);
        }
    };

    YAML::Node topNode;

    try
    {
        topNode = YAML::Load(pYAMLString);
    }
    catch (const YAML::ParserException&)
    {
        throw std::runtime_error("Could not successfully parse YAML content.");
    }

    ptree tTree;

    fillTree(tTree, topNode);

    return tTree;
}

/*!
 * \brief Parse a sequence of unsigned integers from YAML format.
 *
 * Assumes that \p pYAMLString is a YAML document with a single top node of type sequence (or map)
 * with only scalar elements of type unsigned integer. The document is parsed by propertyTreeFromYAML()
 * and then the unsigned integer elements are collected in declaration order and returned as a vector.
 *
 * \param pYAMLString The unsigned integer sequence as YAML sequence/map.
 * \return A vector filled with the parsed numbers (in declaration order).
 */
std::vector<std::uint64_t> uintSeqFromYAML(const std::string& pYAMLString)
{
    boost::property_tree::ptree valsTree = propertyTreeFromYAML(pYAMLString);

    LayerConfig valsConf = LayerConfig(valsTree);

    std::vector<std::uint64_t> valsVec;
    valsVec.reserve(valsTree.size());

    for (auto [key, val] : valsTree)
    {
        (void)val;

        try
        {
            if (!valsConf.contains(LayerConfig::fromYAML("{\"" + key + "\": uint}"), true))
                throw std::runtime_error("Value must be unsigned.");
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error(std::string("Invalid value in YAML unsigned value sequence: ") + exc.what());
        }

        valsVec.push_back(valsConf.getUInt(key));
    }

    return valsVec;
}

} // namespace casil::Auxil
