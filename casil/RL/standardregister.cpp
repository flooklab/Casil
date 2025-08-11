/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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
//
//  This file has been directly adapted from the corresponding code of
//  the basil software, which itself is covered by the following license:
//
//      Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn
//
//      All rights reserved.
//
//      Redistribution and use in source and binary forms, with or without
//      modification, are permitted provided that the following conditions are
//      met:
//
//       *  Redistributions of source code must retain the above copyright notice,
//          this list of conditions and the following disclaimer.
//
//       *  Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//
//       *  Neither the name of the copyright holder nor the names of its
//          contributors may be used to endorse or promote products derived from
//          this software without specific prior written permission.
//
//      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include <casil/RL/standardregister.h>

#include <casil/bytes.h>

#include <stdexcept>

namespace
{

using RegField = casil::Layers::RL::StandardRegister::RegField;
using BoolRef = casil::Layers::RL::StandardRegister::BoolRef;

/*
 * TODO
 */
std::vector<BoolRef> createBitRefs(boost::dynamic_bitset<>& pBits, std::uint64_t pSize, std::uint64_t pOffs)
{
    std::vector<BoolRef> retVal;
    retVal.reserve(pSize);
    for (std::size_t i = pOffs-(pSize-1); i <= pOffs; ++i)
        retVal.emplace_back(pBits, i);
    return retVal;
}

/*
 * TODO
 */
std::vector<BoolRef> createBitRefs(const RegField& pParent, std::uint64_t pSize, std::uint64_t pOffs)
{
    std::vector<BoolRef> retVal;
    retVal.reserve(pSize);
    for (std::size_t i = pOffs-(pSize-1); i <= pOffs; ++i)
        retVal.emplace_back(pParent, i);
    return retVal;
}

} // namespace

using casil::Layers::RL::StandardRegister;

CASIL_REGISTER_REGISTER_CPP(StandardRegister)
CASIL_REGISTER_REGISTER_ALIAS("StdRegister")

//

/*!
 * \brief Constructor.
 *
 * \todo ...
 *
 * \param pName Component instance name.
 * \param pDriver %Driver instance to be used.
 * \param pConfig Component configuration.
 */
StandardRegister::StandardRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig) :
    Register(typeName, std::move(pName), pDriver, std::move(pConfig), LayerConfig::fromYAML("{size: uint}")),
    size(config.getUInt("size", 0)),
    data(size, 0)
{
    //TODO process "init", "auto_start", "bit_order" (field)

    if (size == 0)
        throw std::runtime_error("Invalid register size set for " + getSelfDescription() + ".");

    fields.data() = std::make_shared<RegField>(data, "", size, size-1);

    const boost::property_tree::ptree fieldsConfig = config.getRawTreeAt("fields");

    populateFieldTree(fields, fieldsConfig, "fields");
}

//Public

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pFieldPath
 * \return
 */
const StandardRegister::RegField& StandardRegister::operator[](const std::string& pFieldPath) const
{
    try
    {
        return *(fields.get_child(pFieldPath).data());
    }
    catch (const boost::property_tree::ptree_bad_path&)
    {
        throw std::invalid_argument("The register field \"" + pFieldPath + "\" is not available for " + getSelfDescription() + ".");
    }
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pIdx
 * \return
 */
const StandardRegister::BoolRef& StandardRegister::operator[](const std::size_t pIdx) const
{
    if (pIdx >= size)
        throw std::invalid_argument("Index exceeds register size for " + getSelfDescription() + ".");

    return (*(fields.data()))[pIdx];
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
const StandardRegister::RegField& StandardRegister::root() const
{
    return *(fields.data());
}

//

/*!
 * \brief Get the size of the register.
 *
 * \return Number of register bits.
 */
std::uint64_t StandardRegister::getSize() const
{
    return size;
}

//Private

/*!
 * \copybrief Register::initImpl()
 *
 * ...
 *
 * \todo ...
 *
 * \return ...
 */
bool StandardRegister::initImpl()
{
    //TODO
    return true;
}

/*!
 * \copybrief Register::closeImpl()
 *
 * ...
 *
 * \todo ...
 *
 * \return ...
 */
bool StandardRegister::closeImpl()
{
    //TODO
    return true;
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pFieldTree
 * \param pConfTree
 * \param pParentKey
 */
void StandardRegister::populateFieldTree(FieldTree& pFieldTree, const boost::property_tree::ptree& pConfTree,
                                         const std::string& pParentKey) const
{
    if (pConfTree.data() != "")
        throw std::runtime_error("Invalid register fields configuration for " + getSelfDescription() + ".");

    for (const auto& [key, field] : pConfTree)
    {
        if (field.data() != "")
            throw std::runtime_error("Invalid register field configuration for " + getSelfDescription() + ".");

        for (const auto& [fieldKey, fieldVal] : field)
        {
            (void)fieldVal;
            if (fieldKey != "name" && fieldKey != "size" && fieldKey != "offset" && fieldKey != "fields" && fieldKey != "repeat")
                throw std::runtime_error("Unknown key \"" + fieldKey + "\" in register field definition for " + getSelfDescription() + ".");
        }

        if (field.find("name") == field.not_found() || field.find("size") == field.not_found() ||
            field.find("offset") == field.not_found())
        {
            throw std::runtime_error("Missing key in register field definition for " + getSelfDescription() + ".");
        }

        const bool repsDefined = (field.find("repeat") != field.not_found());

        if (!field.get_child("name").empty() || field.get_child("name").data() == "" ||
            !field.get_child("size").empty() || field.get_child("size").data() == "" ||
            !field.get_child("offset").empty() || field.get_child("offset").data() == "" ||
            (repsDefined && (!field.get_child("repeat").empty() || field.get_child("repeat").data() == "")))
        {
            throw std::runtime_error("Invalid register field configuration for " + getSelfDescription() + ".");
        }

        const std::string fullKey = pParentKey + "." + key;

        const std::string tName = config.getStr(fullKey + ".name");
        const std::uint64_t tSize = config.getUInt(fullKey + ".size", 0);
        const std::uint64_t tOffs = config.getUInt(fullKey + ".offset", 0); //TODO need check if conversion OK (size 0, repeat 0 excl. anyway)
        const std::uint64_t tReps = repsDefined ? config.getUInt(fullKey + ".repeat", 0) : 1;

        if (tSize == 0)
            throw std::runtime_error("Zero size set for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (tReps == 0)
            throw std::runtime_error("Zero repetitions set for register field \"" + tName + "\" of " + getSelfDescription() + ".");

        if (pFieldTree.find(tName) != pFieldTree.not_found())
            throw std::runtime_error("Field with name \"" + tName + "\" is defined multiple times for " + getSelfDescription() + ".");

        const RegField& parentField = *(pFieldTree.data());

        if ((tSize*tReps > tOffs+1) || (tOffs >= parentField.getSize()))
            throw std::runtime_error("Register field \"" + tName + "\" exceeds parent field's extent for " + getSelfDescription() + ".");

        FieldTree tSubTree;

        if (tReps > 1)  //Need to (iteratively) add nested layer of fields to reflect and enable access to individual repetitions
        {
            tSubTree.data() = std::make_shared<RegField>(parentField, tName, tSize*tReps, tOffs);

            const RegField& parentFieldForReps = *(tSubTree.data());

            for (std::uint64_t i = 0; i < tReps; ++i)
            {
                const std::string tSubName = "#" + std::to_string(i);

                FieldTree tSubSubTree;

                tSubSubTree.data() = std::make_shared<RegField>(parentFieldForReps, tSubName, tSize, tSize*(tReps-i)-1);

                //Recurse, if any sub-fields specified
                if (field.find("fields") != field.not_found())
                    populateFieldTree(tSubSubTree, field.get_child("fields"), fullKey + ".fields");

                //Can make current field repetition make aware of its immediate childs after recursion
                std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> childFieldRefs;
                for (const auto& [subSubFieldName, subSubField] : tSubSubTree)
                    childFieldRefs.emplace(subSubFieldName, *(subSubField.data()));
                tSubSubTree.data()->setChildFields(std::move(childFieldRefs));

                tSubTree.add_child(tSubName, tSubSubTree);
            }

            //Can make current field make aware of its repetition-childs after iteration
            std::vector<std::pair<std::string, const std::reference_wrapper<const RegField>>> childFieldRefs;
            childFieldRefs.reserve(tReps);
            for (const auto& [subFieldName, subField] : tSubTree)
                childFieldRefs.emplace_back(subFieldName, *(subField.data()));
            tSubTree.data()->setChildFields(childFieldRefs);
        }
        else
        {
            tSubTree.data() = std::make_shared<RegField>(parentField, tName, tSize, tOffs);

            //Recurse, if any sub-fields specified
            if (field.find("fields") != field.not_found())
                populateFieldTree(tSubTree, field.get_child("fields"), fullKey + ".fields");

            //Can make current field make aware of its immediate childs after recursion
            std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> childFieldRefs;
            for (const auto& [subFieldName, subField] : tSubTree)
                childFieldRefs.emplace(subFieldName, *(subField.data()));
            tSubTree.data()->setChildFields(std::move(childFieldRefs));
        }

        pFieldTree.add_child(tName, tSubTree);
    }
}

//StandardRegister::BoolRef

using BoolRef = StandardRegister::BoolRef;

/*!
 * \brief Constructor.
 *
 * \todo ...
 *
 * \param pBits
 * \param pIdx
 */
BoolRef::BoolRef(boost::dynamic_bitset<>& pBits, const std::size_t pIdx) :
    dataField(pBits),
    idx(pIdx)
{
}

/*!
 * \brief Constructor.
 *
 * \todo ...
 *
 * \param pParent
 * \param pIdx
 */
BoolRef::BoolRef(const RegField& pParent, const std::size_t pIdx) :
    dataField(pParent),
    idx(pIdx)
{
}

//Public

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pValue
 * \return
 */
bool BoolRef::operator=(const bool pValue) const
{
    if (std::holds_alternative<const BitsetRef>(dataField))
        std::get<const BitsetRef>(dataField).get().operator[](idx) = pValue;
    else
        std::get<const FieldRef>(dataField).get().operator[](idx) = pValue;

    return pValue;
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 */
BoolRef::operator bool() const
{
    if (std::holds_alternative<const BitsetRef>(dataField))
        return std::get<const BitsetRef>(dataField).get().operator[](idx);
    else
        return std::get<const FieldRef>(dataField).get().operator[](idx);
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
bool BoolRef::get() const
{
    return (operator bool());
}

//StandardRegister::RegField

using RegField = StandardRegister::RegField;

/*!
 * \brief Constructor.
 *
 * \todo ...
 *
 * \param pBits
 * \param pName
 * \param pSize
 * \param pOffs
 */
RegField::RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs) :
    name(pName),
    size(pSize),
    offs(pOffs),
    dataRefs(::createBitRefs(pBits, size, offs)),
    childFields(),
    repetitionKeys()
{
}

/*!
 * \brief Constructor.
 *
 * \todo ...
 *
 * \param pParent
 * \param pName
 * \param pSize
 * \param pOffs
 */
RegField::RegField(const RegField& pParent, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs) :
    name(pName),
    size(pSize),
    offs(pOffs),
    dataRefs(::createBitRefs(pParent, size, offs)),
    childFields(),
    repetitionKeys()
{
}

//Public

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pValue
 * \return
 */
std::uint64_t RegField::operator=(const std::uint64_t pValue) const
{
    *this = Bytes::bitsetFromBytes(Bytes::composeByteVec(true, pValue), size);
    return pValue;
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pBits
 * \return
 */
const boost::dynamic_bitset<>& RegField::operator=(const boost::dynamic_bitset<>& pBits) const
{
    if (pBits.size() != size)
        throw std::invalid_argument("Wrong number of bits for register field \"" + name + "\".");

    for (std::size_t i = 0; i < size; ++i)
        dataRefs[i] = pBits[i];

    return pBits;
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 */
RegField::operator std::uint64_t() const
{
    return Bytes::composeUInt64(Bytes::bytesFromBitset((operator boost::dynamic_bitset<>()), 8), true);
}

/*!
 * \brief TODO
 *
 * \todo ...
 */
RegField::operator boost::dynamic_bitset<>() const
{
    boost::dynamic_bitset retVal(size);

    for (std::size_t i = 0; i < size; ++i)
        retVal[i] = dataRefs[i].get();

    return retVal;
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
std::uint64_t RegField::toUInt() const
{
    return (operator std::uint64_t());
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
boost::dynamic_bitset<> RegField::toBits() const
{
    return (operator boost::dynamic_bitset<>());
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pFieldName
 * \return
 */
const RegField& RegField::operator[](const std::string_view pFieldName) const
{
    const auto it = childFields.find(pFieldName);

    if (it == childFields.end())
        throw std::invalid_argument("Register field \"" + name + "\" has no sub-field \"" + std::string(pFieldName) + "\".");

    return it->second;
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pIdx
 * \return
 */
const BoolRef& RegField::operator[](const std::size_t pIdx) const
{
    if (pIdx >= size)
        throw std::invalid_argument("Index " + std::to_string(pIdx) + " is out of range for register field \"" + name + "\".");

    return dataRefs[pIdx];
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pFieldRepIdx
 * \return
 */
const RegField& RegField::n(const std::size_t pFieldRepIdx) const
{
    if (pFieldRepIdx >= repetitionKeys.size())
    {
        if (repetitionKeys.size() == 0)
            throw std::runtime_error("Register field \"" + name + "\" has no repetitions.");
        else
            throw std::invalid_argument("Register field \"" + name + "\" has no repetition with index " + std::to_string(pFieldRepIdx) + ".");
    }

    const auto it = childFields.find(repetitionKeys[pFieldRepIdx]);

    if (it == childFields.end())
        throw std::runtime_error("Could not find register field for given repetition index. THIS SHOULD NEVER HAPPEN!");

    return it->second;
}

//

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
std::uint64_t RegField::getSize() const
{
    return size;
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \return
 */
std::uint64_t RegField::getOffset() const
{
    return offs;
}

//Private

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pChildFields
 */
void RegField::setChildFields(std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> pChildFields)
{
    childFields.swap(pChildFields);
    repetitionKeys.clear();     //Should not be needed in practice, just to make really sure this was not set before by other overload
}

/*!
 * \brief TODO
 *
 * \todo ...
 *
 * \param pFieldReps
 */
void RegField::setChildFields(const std::vector<std::pair<std::string, const std::reference_wrapper<const RegField>>>& pFieldReps)
{
    childFields.clear();
    repetitionKeys.clear();

    for (std::size_t i = 0; i < pFieldReps.size(); ++i)
    {
        childFields.insert(pFieldReps[i]);
        repetitionKeys.push_back(pFieldReps[i].first);
    }
}
