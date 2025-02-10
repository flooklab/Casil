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

#include <casil/layerconfig.h>

#include <casil/auxil.h>

#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/convert.h>  //Keep include order!
#include <yaml-cpp/node/impl.h>     //Keep include order!

#include <functional>
#include <type_traits>
#include <utility>
#include <sstream>

namespace
{

using boost::property_tree::ptree;
using boost::property_tree::ptree_bad_path;

/*
 * Parses and returns the value sequence stored in the configuration tree 'pTree'
 * at branch location 'pKey' with sequence elements converted to integer type 'T'.
 *
 * Returns 'pDefault' if the conversion fails or if 'pKey' is not found.
 */
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
 * Loads \p pTree as the configuration tree.
 *
 * \param pTree Component configuration tree with format according to Auxil::propertyTreeFromYAML.
 */
LayerConfig::LayerConfig(const boost::property_tree::ptree& pTree) :
    tree(pTree)
{
}

//Public

/*!
 * \brief Equality operator.
 *
 * Compares the configuration tree to the one of \p pOther. See also \c boost::property_tree::ptree::operator==().
 *
 * \param pOther Other component configuration to compare to.
 * \return True if the structures and all elements of the configuration trees are equal.
 */
bool LayerConfig::operator==(const LayerConfig& pOther) const
{
    return (tree == pOther.tree);
}

//

/*!
 * \brief Check the configuration tree structure (and value types).
 *
 * Checks if the configuration tree contains every key/branch that is in the tree of \p pOther.
 * If \p pCheckTypes is true the values at the tree's branch tips will be checked against the
 * type descriptions from the corresponding branch tips of \p pOther by trying to convert
 * the stored strings to the respective types in accordance with the YAML specification.
 *
 * Type descriptions can be any of the following:
 * - "bool": Boolean value
 * - "int": Integer value
 * - "uint": Unsigned integer value (max. 64 bit)
 * - "double": Floating point value with double precision
 * - "float": Equivalent to "double"
 * - "string": Anything else, as values are stored as strings anyway (i.e. this type check will never fail)
 * - "byteSeq": Must not be a scalar value but a sequence-like sub-tree with scalar elements of 8 bit unsigned integer type
 * - "uintSeq" Must not be a scalar value but a sequence-like sub-tree with scalar elements of max. 64 bit unsigned integer type
 * - Everything else is treated as "string"
 *
 * This function returns false if a branch from \p pOther is not found or if a type check fails.
 *
 * Example with YAML code:
 * <tt>{init: {port: /dev/ttyUSB1, baudrate: 19200, nested: [1, 2, 3]}}</tt> contains
 * <tt>{init: {port: string, nested: byteSeq}}</tt> with type check enabled.
 *
 * \param pOther Reference configuration to compare to.
 * \param pCheckTypes Check type conversion of stored values.
 * \return True if \p pOther is fully contained in the configuration tree.
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
 * Gets the configuration value stored in the tree at branch location \p pKey converted to boolean type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
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
 * Gets the configuration value stored in the tree at branch location \p pKey converted to integer type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
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
 * Gets the configuration value stored in the tree at branch location \p pKey converted to unsigned integer type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
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
 * Gets the configuration value stored in the tree at branch location \p pKey converted to floating point type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
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
 * Gets the configuration value stored in the tree at branch location \p pKey as string.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
 */
std::string LayerConfig::getStr(const std::string& pKey, const std::string& pDefault) const
{
    return tree.get<std::string>(pKey, pDefault);
}

//

/*!
 * \brief Get an 8 bit unsigned integer sequence from the configuration tree.
 *
 * Gets the configuration value sequence stored in the tree at branch location \p pKey
 * with sequence elements converted to 8 bit unsigned integer type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
 */
std::vector<std::uint8_t> LayerConfig::getByteSeq(const std::string& pKey, std::vector<std::uint8_t> pDefault) const
{
    return ::geTSeq<std::uint8_t>(tree, pKey, std::move(pDefault));
}

/*!
 * \brief Get a 64 bit unsigned integer sequence from the configuration tree.
 *
 * Gets the configuration value sequence stored in the tree at branch location \p pKey
 * with sequence elements converted to 64 bit unsigned integer type.
 *
 * Returns \p pDefault if the conversion fails or if \p pKey is not found.
 *
 * \param pKey Value location as individual branch keys that are connected with full stops (".").
 * \param pDefault Default value to return on error.
 * \return Converted value at \p pKey or \p pDefault in case of an error.
 */
std::vector<std::uint64_t> LayerConfig::getUIntSeq(const std::string& pKey, std::vector<std::uint64_t> pDefault) const
{
    return ::geTSeq<std::uint64_t>(tree, pKey, std::move(pDefault));
}

//

/*!
 * \brief Format the configuration tree content as human-readable string.
 *
 * \return The configuration tree as a string (formatted similar to the represented YAML document).
 */
std::string LayerConfig::toString() const
{
    using boost::property_tree::ptree;

    std::ostringstream ostrm;

    std::function<void(const ptree&, int)> printTree = [&printTree, &ostrm](const ptree& pTree, const int pLevel) -> void
    {
        const std::string indent(4*pLevel, ' ');

        if (pLevel > 0)
        {
            if (pTree.data() != "")
                ostrm<<": "<<pTree.data()<<"\n";
            else
                ostrm<<":\n";
        }

        for (const auto& it : pTree)
        {
            ostrm<<indent<<it.first;
            printTree(it.second, pLevel+1);
        }
    };

    printTree(tree, 0);

    return ostrm.str();
}

//

/*!
 * \brief Create a configuration object from YAML format.
 *
 * Constructs a LayerConfig instance from a configuration tree obtained from Auxil::propertyTreeFromYAML().
 * See also LayerConfig(const boost::property_tree::ptree&).
 *
 * \param pYAMLString The YAML document to be parsed.
 * \return The according layer component configuration object.
 */
LayerConfig LayerConfig::fromYAML(const std::string& pYAMLString)
{
    return LayerConfig(Auxil::propertyTreeFromYAML(pYAMLString));
}
