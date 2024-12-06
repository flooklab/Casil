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

#include <casil/layerconfig.h>

#include <casil/auxil.h>

#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/convert.h>  //Keep include order!
#include <yaml-cpp/node/impl.h>     //Keep include order!

#include <functional>
#include <type_traits>
#include <utility>

namespace
{

using boost::property_tree::ptree;
using boost::property_tree::ptree_bad_path;

template<typename T>
    requires std::is_integral_v<T>
std::vector<T> geTSeq(const ptree& pTree, const std::string& pKey, std::vector<T>&& pDefault)
{
    try
    {
        const ptree& subTree = pTree.get_child(pKey);

        if (subTree.data() != "")
            return pDefault;

        std::vector<T> retSeq;
        retSeq.reserve(subTree.size());

        for (const auto& [seqKey, seqVal] : subTree)
        {
            (void)seqKey;

            if (seqVal.data() == "" || !seqVal.empty())
                return pDefault;

            try
            {
                YAML::Node tNode(YAML::NodeType::Scalar);
                tNode = seqVal.data();
                retSeq.push_back(tNode.as<T>());
            }
            catch (const YAML::BadConversion&)
            {
                return pDefault;
            }
        }

        return retSeq;
    }
    catch (const ptree_bad_path&)
    {
        return pDefault;
    }
}

} // namespace

using casil::LayerConfig;

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pTree
 */
LayerConfig::LayerConfig(const boost::property_tree::ptree& pTree) :
    tree(pTree)
{
}

//Public

/*!
 * \brief Equality operator.
 *
 * \todo Detailed doc
 *
 * \param pOther
 * \return
 */
bool LayerConfig::operator==(const LayerConfig& pOther) const
{
    return (tree == pOther.tree);
}

//

/*!
 * \brief Check the configuration tree structure (and value types).
 *
 * \todo Detailed doc
 *
 * \param pOther
 * \param pCheckTypes
 * \return
 */
bool LayerConfig::contains(const LayerConfig& pOther, const bool pCheckTypes) const
{
    using boost::property_tree::ptree;

    std::function<bool(const ptree&, const ptree&)> checkSubTree = [&checkSubTree, pCheckTypes](const ptree& pRefTree,
                                                                                                const ptree& pCheckTree) -> bool
    {
        for (const auto& [key, val] : pRefTree)
        {
            if (pCheckTree.find(key) != pCheckTree.not_found())
            {
                if (!checkSubTree(val, pCheckTree.find(key)->second))
                    return false;
            }
            else
                return false;
        }

        if (pCheckTypes && pRefTree.empty() && pRefTree.data() != "")
        {
            if (pRefTree.data() == "uintSeq" || pRefTree.data() == "byteSeq")
            {
                if (pCheckTree.data() != "")
                    return false;

                for (const auto& [seqKey, seqVal] : pCheckTree)
                {
                    (void)seqKey;

                    if (seqVal.data() == "" || !seqVal.empty())
                        return false;

                    try
                    {
                        YAML::Node tNode(YAML::NodeType::Scalar);
                        tNode = seqVal.data();

                        if (pRefTree.data() == "uintSeq")
                            tNode.as<std::uint64_t>();
                        else if (pRefTree.data() == "byteSeq")
                            tNode.as<std::uint8_t>();
                    }
                    catch (const YAML::BadConversion&)
                    {
                        return false;
                    }
                }
            }
            else if (!pCheckTree.empty())   //Already/only handled by "uintSeq" and "byteSeq" cases above
            {
                return false;
            }
            else if (pRefTree.data() != "string")
            {
                try
                {
                    YAML::Node tNode(YAML::NodeType::Scalar);
                    tNode = pCheckTree.data();

                    if (pRefTree.data() == "bool")
                        tNode.as<bool>();
                    else if (pRefTree.data() == "int")
                        tNode.as<int>();
                    else if (pRefTree.data() == "uint")
                        tNode.as<std::uint64_t>();
                    else if (pRefTree.data() == "double" || pRefTree.data() == "float")
                        tNode.as<double>();
                }
                catch (const YAML::BadConversion&)
                {
                    return false;
                }
            }
        }

        return true;
    };

    return checkSubTree(pOther.tree, tree);
}

//

/*!
 * \brief Get a boolean configuration value.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
bool LayerConfig::getBool(const std::string &pKey, const bool pDefault) const
{
    try
    {
        YAML::Node tNode(YAML::NodeType::Scalar);
        tNode = getStr(pKey, "");
        return tNode.as<bool>();
    }
    catch (const YAML::BadConversion&)
    {
        return pDefault;
    }
}

/*!
 * \brief Get a (signed) integer configuration value.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
int LayerConfig::getInt(const std::string& pKey, const int pDefault) const
{
    try
    {
        YAML::Node tNode(YAML::NodeType::Scalar);
        tNode = getStr(pKey, "");
        return tNode.as<int>();
    }
    catch (const YAML::BadConversion&)
    {
        return pDefault;
    }
}

/*!
 * \brief Get an unsigned integer configuration value.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
std::uint64_t LayerConfig::getUInt(const std::string& pKey, const std::uint64_t pDefault) const
{
    try
    {
        YAML::Node tNode(YAML::NodeType::Scalar);
        tNode = getStr(pKey, "");
        return tNode.as<std::uint64_t>();
    }
    catch (const YAML::BadConversion&)
    {
        return pDefault;
    }
}

/*!
 * \brief Get a floating point configuration value.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
double LayerConfig::getDbl(const std::string& pKey, const double pDefault) const
{
    try
    {
        YAML::Node tNode(YAML::NodeType::Scalar);
        tNode = getStr(pKey, "");
        return tNode.as<double>();
    }
    catch (const YAML::BadConversion&)
    {
        return pDefault;
    }
}

/*!
 * \brief Get a string-type configuration value.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
std::string LayerConfig::getStr(const std::string& pKey,
                                const std::string pDefault) const   // cppcheck-suppress passedByValue symbolName=pDefault
{
    return tree.get<std::string>(pKey, pDefault);
}

//

/*!
 * \brief Get an 8 bit unsigned integer sequence from the configuration tree.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
std::vector<std::uint8_t> LayerConfig::getByteSeq(const std::string& pKey, std::vector<std::uint8_t> pDefault) const
{
    return ::geTSeq<std::uint8_t>(tree, pKey, std::move(pDefault));
}

/*!
 * \brief Get a 64 bit unsigned integer sequence from the configuration tree.
 *
 * \todo Detailed doc
 *
 * \param pKey
 * \param pDefault
 * \return
 */
std::vector<std::uint64_t> LayerConfig::getUIntSeq(const std::string& pKey, std::vector<std::uint64_t> pDefault) const
{
    return ::geTSeq<std::uint64_t>(tree, pKey, std::move(pDefault));
}

//

/*!
 * \brief Create a configuration object from YAML format.
 *
 * \todo Detailed doc
 *
 * \param pYAMLString
 * \return
 */
LayerConfig LayerConfig::fromYAML(const std::string& pYAMLString)
{
    return LayerConfig(Auxil::propertyTreeFromYAML(pYAMLString));
}
