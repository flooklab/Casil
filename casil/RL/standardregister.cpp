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
 * Creates and returns a 'pSize' long vector of proxy class instances for bit manipulation of a contiguous
 * range of 'pSize' bits that are stored in 'pBits' at a most significant bit offset of 'pOffs'.
 * The indexing is hence such that returnValue[(pSize-1):0] ^= pBits[pOffs:(pOffs-(pSize-1))].
 * Boundaries are not checked here.
 */
std::vector<BoolRef> createBitRefs(boost::dynamic_bitset<>& pBits, const std::uint64_t pSize, const std::uint64_t pOffs)
{
    std::vector<BoolRef> retVal;
    retVal.reserve(pSize);
    for (std::size_t i = pOffs-(pSize-1); i <= pOffs; ++i)
        retVal.emplace_back(pBits, i);
    return retVal;
}

/*
 * Creates and returns a 'pSize' long vector of proxy class instances for bit manipulation of a contiguous range of
 * 'pSize' bits from (and in turn indirectly referenced by) 'pParent' at a most significant bit offset of 'pOffs'.
 * The indexing is hence such that returnValue[(pSize-1):0] ^= pParent[pOffs:(pOffs-(pSize-1))].
 * Boundaries are not checked here.
 */
std::vector<BoolRef> createBitRefs(const RegField& pParent, const std::uint64_t pSize, const std::uint64_t pOffs)
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
 * Gets the mandatory "size" value from \p pConfig (unsigned integer value), which defines the number of
 * register bits, constructs an accordingly sized bit storage and creates a proxy class tree for a simple
 * structured access to any named register \e fields as defined in the optional "fields" sequence in \p pConfig.
 *
 * Any field in that configuration sequence is a map consisting of at least a field name (key "name"), bit offset (key "offset")
 * and bit size (key "size"). The offset denotes the index that the field's most significant bit has in the parent field
 * (assuming the most significant bit always has the highest index, i.e. the register and each of its fields are ordered
 * with least significant bit first). The field name must not begin with '#' and must not contain any periods ('.').
 *
 * Every field can optionally define its own nested field sequence (key "fields") to further define sub-fields,
 * which all have to be fully contained within the defining field.
 *
 * If the unsigned integer value "repeat" is defined and larger than 1, an intermediate layer (between the field and possibly
 * sub-fields) of sub-fields is automatically inserted, representing identical (except bit shifted) repetitions/copies of the
 * given field configuration including its sub-fields. The actual size of the field then becomes \c repeat \c * \c size, while
 * each of the repetitions has a size of \c size and has an accordingly shifted offset with respect to the full repeated field.
 * The n-th repetition can be accessed via RegField::n() or the regular RegField::operator[](std::string_view) const /
 * StandardRegister::operator[](const std::string&) const using "#0", "#1", etc. as keys.
 * Note that here the zeroth repetition is the one starting at the highest offset (as inherited from basil).
 *
 * \todo bit_order...
 * \todo auto_start...
 * \todo init...
 *
 * \todo add more and describe those exceptions
 *
 * \internal \sa populateFieldTree() \endinternal
 *
 * \throws std::runtime_error If "size" is not defined or set to zero.
 * \throws std::runtime_error If the "fields" sequence from \p pConfig has an overall invalid structure (see above).
 * \throws std::runtime_error If a field node or a "fields" node contains data instead of just further child nodes.
 * \throws std::runtime_error If a field node definition contains an unknown key.
 * \throws std::runtime_error If a field node definition is missing an essential value (need "name", "size", "offset").
 * \throws std::runtime_error If one of the "value"-keys (such as "name", "size", etc.) in a field node definition does not
 *                            hold a \e value or has child nodes.
 * \throws std::runtime_error If a field size is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field repetition count is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field name is invalid (contains '.' or starts with '#').
 * \throws std::runtime_error If the same field name is defined multiple times within a group of child fields.
 * \throws std::runtime_error If a field exceeds the parent field's extent.
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
 * \brief Access a specific register field.
 *
 * Returns a proxy reference to the register field at node \p pFieldPath in the field tree/hierarchy.
 * The different names of fields and their sub-fields need to be concatenated by periods ('.') and field
 * repetitions are selected by names "#0", "#1", etc. (e.g. "TOP_LEVEL_FIELD_1.SOME_NESTED_COL.#47.CAL").
 *
 * \throws std::invalid_argument If \p pFieldPath does not point to an existing register field.
 *
 * \param pFieldPath Path to the desired field.
 * \return Proxy class instance for register field at \p pFieldPath.
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
 * \brief Access a specific bit in the register.
 *
 * \throws std::invalid_argument If \p pIdx exceeds the register size.
 *
 * \param pIdx Bit number, assuming least significant bit first.
 * \return Proxy class instance for register bit \p pIdx.
 */
const StandardRegister::BoolRef& StandardRegister::operator[](const std::size_t pIdx) const
{
    if (pIdx >= size)
        throw std::invalid_argument("Index exceeds register size for " + getSelfDescription() + ".");

    return (*(fields.data()))[pIdx];
}

//

/*!
 * \brief Get the root field node.
 *
 * \return The top-level register field, which represents the \e entire register.
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
 * \return True if successful.
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
 * \return True if successful.
 */
bool StandardRegister::closeImpl()
{
    //TODO
    return true;
}

//

/*!
 * \brief Populate the register field tree by recursing through the field configuration.
 *
 * This is a recursive helper function for StandardRegister() in order to populate the register field
 * tree \p pFieldTree according to the field configuration \p pConfTree. The field configuration describes,
 * which named \e fields the register (and which nested fields one of the fields) is supposed to have.
 * It must adhere to the following structure:
 *
 * - \c pConfTree: no %data() at node
 *  - field1: no %data() at node
 *   - name: "FIELD_NAME"
 *   - size: 12
 *   - offset: 20
 *   - fields: no %data() at node
 *    - field11: no %data() at node
 *     - name: "Nested-FIELD_NAME"
 *     - size: 2
 *     - offset: 11
 *    - field12: no %data() at node
 *     - name: "Nested-FIELD_NAME-2"
 *     - size: 10
 *     - offset: 9
 *     - bit_order: no %data() at node
 *      - bit9idx: 5
 *      - ...
 *      - bit0idx: 8
 *  - field2: no %data() at node
 *   - name: "OTHER_FIELD_NAME"
 *   - size: 2
 *   - offset: 7
 *   - repeat: 3
 *
 * As can be seen, the "repeat" and "bit_order" entries are optional. For more information on \p pConfTree,
 * allowed values and their effects please refer to StandardRegister().
 *
 * This function inserts a proxy instance as child into \p pFieldTree for every field description from \p pConfTree, and for every
 * child that has a nested "fields" sequence, does that recursively for the child branch. Proxy instance here means that the fields in
 * \p pFieldTree do not actually store the data but merely refer to the bits from the respectively parent field. Hence, when initially
 * calling this function, \p pFieldTree already must have such a field proxy at the root node that does represent the whole register
 * and points to the actual register data/bits storage. \p pFieldTree is otherwise expected to be empty. See also StandardRegister().
 *
 * \p pParentKey must be the path in this instance's LayerConfig (see StandardRegister()) that points to \p pConfTree,
 * which is used to retrieve formatted data for each field.
 *
 * Note again: For more information see StandardRegister().
 *
 * \throws std::runtime_error If \p pConfTree has an overall invalid structure (see above).
 * \throws std::runtime_error If a field node or a "fields" node contains data instead of just further child nodes.
 * \throws std::runtime_error If a field node definition contains an unknown key.
 * \throws std::runtime_error If a field node definition is missing an essential value (need "name", "size", "offset").
 * \throws std::runtime_error If one of the "value"-keys (such as "name", "size", etc.) in a field node definition does not
 *                            hold a \e value or has child nodes.
 * \throws std::runtime_error If a field size is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field repetition count is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field name is invalid (contains '.' or starts with '#').
 * \throws std::runtime_error If the same field name is defined multiple times within a group of child fields.
 * \throws std::runtime_error If a field exceeds the parent field's extent.
 * \throws std::runtime_error Possibly/effectively if \p pParentKey is wrong (might cause on of the other exceptions).
 *
 * \param pFieldTree Tree to be filled with proxies for every register field as defined in \p pConfTree.
 * \param pConfTree Configuration tree describing the register fields structure/hierarchy.
 * \param pParentKey Path at which \p pConfTree can be found in the layer component's LayerConfig instance.
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

        if (tName.find('.') != tName.npos || tName.starts_with('#'))
            throw std::runtime_error("Invalid name set for register field \"" + tName + "\" of " + getSelfDescription() + ".");
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
 * Binds the instance's bit access/manipulation functions to the bit \p pIdx of bitset \p pBits.
 *
 * \throws std::invalid_argument If \p pIdx exceeds size of \p pBits.
 *
 * \param pBits Bitset that holds the referenced bit.
 * \param pIdx Index of the referenced bit.
 */
BoolRef::BoolRef(boost::dynamic_bitset<>& pBits, const std::size_t pIdx) :
    dataField(pBits),
    idx(pIdx)
{
    if  (pIdx >= pBits.size())
        throw std::invalid_argument("Index exceeds size of referenced bitset.");
}

/*!
 * \brief Constructor.
 *
 * Binds the instance's bit access/manipulation functions to the bit \p pIdx of register field \p pParent.
 *
 * \throws std::invalid_argument If \p pIdx exceeds size of \p pParent.
 *
 * \param pParent Register field that itself references the referenced bit.
 * \param pIdx Index of the referenced bit.
 */
BoolRef::BoolRef(const RegField& pParent, const std::size_t pIdx) :
    dataField(pParent),
    idx(pIdx)
{
    if  (pIdx >= pParent.getSize())
        throw std::invalid_argument("Index exceeds size of referenced field.");
}

//Public

/*!
 * \brief Assign a value to the referenced bit.
 *
 * Sets the referenced bit's value to \p pValue through the interface of the referenced parent.
 *
 * \param pValue Value to be set.
 * \return \p pValue.
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
 * \brief Get the value of the referenced bit.
 *
 * Returns the current value of the referenced bit as determined through the interface of the referenced parent.
 *
 * \return Current value.
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
 * \brief Get the value of the referenced bit.
 *
 * \copydetails BoolRef::operator bool()
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
 * Binds the instance's register field access functions to the field (with name \p pName) that corresponds to the sequence of bits
 * within \p pBits with size \p pSize and offset \p pOffs, i.e. <tt>field[(pSize-1):0] = pBits[pOffs:(pOffs-(pSize-1))]</tt>.
 *
 * \throws std::invalid_argument If \p pSize is zero.
 * \throws std::invalid_argument If the field would exceed the extent of \p pBits (given values of \p pSize and \p pOffs).
 *
 * \param pBits Bitset that holds the referenced bit sequence.
 * \param pName Name of the referenced register field.
 * \param pSize Size of the referenced field in number of bits.
 * \param pOffs Index of the referenced field's most significant bit in \p pBits.
 */
RegField::RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs) :
    name(pName),
    size(pSize),
    offs(pOffs),
    parentSize(pBits.size()),
    parentTotalOffs(pBits.size()-1),
    dataRefs(::createBitRefs(pBits, size, offs)),
    childFields(),
    repetitionKeys()
{
    if (size == 0)
        throw std::invalid_argument("Invalid field size (must be larger than zero).");
    if ((size > offs+1) || (offs >= parentSize))
        throw std::invalid_argument("Field exceeds parent bitset's extent.");
}

/*!
 * \brief Constructor.
 *
 * Binds the instance's register field access functions to the field (with name \p pName) that corresponds to the sequence of bits
 * within \p pParent with size \p pSize and offset \p pOffs, i.e. <tt>field[(pSize-1):0] = pParent[pOffs:(pOffs-(pSize-1))]</tt>.
 *
 * \throws std::invalid_argument If \p pSize is zero.
 * \throws std::invalid_argument If the field would exceed the extent of \p pParent (given values of \p pSize and \p pOffs).
 *
 * \param pParent Parent register field that itself references the bits of the referenced bit sequence.
 * \param pName Name of the referenced register field.
 * \param pSize Size of the referenced field in number of bits.
 * \param pOffs Index of the referenced field's most significant bit in \p pParent.
 */
RegField::RegField(const RegField& pParent, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs) :
    name(pName),
    size(pSize),
    offs(pOffs),
    parentSize(pParent.getSize()),
    parentTotalOffs(pParent.getTotalOffset()),
    dataRefs(::createBitRefs(pParent, size, offs)),
    childFields(),
    repetitionKeys()
{
    if (size == 0)
        throw std::invalid_argument("Invalid field size (must be larger than zero).");
    if ((size > offs+1) || (offs >= parentSize))
        throw std::invalid_argument("Field exceeds parent field's extent.");
}

//Public

/*!
 * \brief Assign equivalent integer value to the field.
 *
 * Takes the binary equivalent of \p pValue and assigns it to the bits that are referenced by the field,
 * with the least significant bit first and the assigned sequence being truncated or zero-padded to the
 * field's size at the most significant bit position.
 *
 * See also RegField::operator=(const boost::dynamic_bitset<>& pBits) const.
 *
 * \param pValue Value to be assigned.
 * \return \p pValue.
 */
std::uint64_t RegField::operator=(const std::uint64_t pValue) const
{
    *this = Bytes::bitsetFromBytes(Bytes::composeByteVec(true, pValue), size);
    return pValue;
}

/*!
 * \brief Assign a raw bit sequence to the field.
 *
 * Assigns \p pBits to the bits that are referenced by the field such that <tt>field[(size-1):0] = pBits[(size-1):0]</tt>.
 *
 * \throws std::invalid_argument If the size of \p pBits differs from the field size.
 *
 * \param pBits Bit sequence to be assigned.
 * \return \p pBits.
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
 * \brief Get the integer equivalent of field's content.
 *
 * Interprets the bit sequence that is referenced by the field as an
 * unsigned integer with least significant bit first and returns that number.
 *
 * See also RegField::operator boost::dynamic_bitset<>().
 *
 * \return Unsigned integer value reptesented by the field's bit sequence.
 */
RegField::operator std::uint64_t() const
{
    return Bytes::composeUInt64(Bytes::bytesFromBitset((operator boost::dynamic_bitset<>()), 8), true);
}

/*!
 * \brief Get the field's content as raw bitset.
 *
 * Returns the current state of the bit sequence that is referenced by the field.
 *
 * \return <tt>field[(size-1):0]</tt>.
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
 * \brief Get the integer equivalent of field's content.
 *
 * \copydetails RegField::operator std::uint64_t
 */
std::uint64_t RegField::toUInt() const
{
    return (operator std::uint64_t());
}

/*!
 * \brief Get the field's data as raw bitset.
 *
 * \copydetails RegField::operator boost::dynamic_bitset<>
 */
boost::dynamic_bitset<> RegField::toBits() const
{
    return (operator boost::dynamic_bitset<>());
}

//

/*!
 * \brief Access an immediate child field.
 *
 * Returns a proxy reference to the immediate child register field with name \p pFieldName.
 *
 * Note that field repetitions (see n()) can also be accessed by using the names "#0", "#1", etc.
 *
 * \throws std::invalid_argument If the field does not have an immediate child field with name \p pFieldName.
 *
 * \param pFieldName Name of the desired field.
 * \return Proxy class instance for child field with name \p pFieldName.
 */
const RegField& RegField::operator[](const std::string_view pFieldName) const
{
    const auto it = childFields.find(pFieldName);

    if (it == childFields.end())
        throw std::invalid_argument("Register field \"" + name + "\" has no sub-field \"" + std::string(pFieldName) + "\".");

    return it->second;
}

/*!
 * \brief Access a specific bit in the field.
 *
 * \throws std::invalid_argument If \p pIdx exceeds the register size.
 *
 * \param pIdx Field-local bit number, assuming least significant bit first.
 * \return Proxy class instance for register field's bit \p pIdx.
 */
const BoolRef& RegField::operator[](const std::size_t pIdx) const
{
    if (pIdx >= size)
        throw std::invalid_argument("Index " + std::to_string(pIdx) + " is out of range for register field \"" + name + "\".");

    return dataRefs[pIdx];
}

//

/*!
 * \brief Access the n-th repetition of the field.
 *
 * If the field's configuration specifies a repetition count larger than one, the individual
 * repetitions can be accessed using this function in the form of dedicated child fields.
 * A proxy reference to the <tt>pFieldRepIdx</tt>-th repetition is returned accordingly.
 *
 * Note that here the zeroth repetition is the one starting at the highest bit offset.
 *
 * Also note that field repetitions can in principle also be accessed via RegField::operator[](std::string_view) const.
 *
 * \throws std::runtime_error If the field does not have any (more than one) repetitions.
 * \throws std::invalid_argument If \p pFieldRepIdx exceeds the repetition count.
 *
 * \param pFieldRepIdx Number of the desired field repetition.
 * \return Proxy class instance for field repetition number \p pFieldRepIdx.
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
 * \brief Get the size of the field.
 *
 * \return Field's size in number of bits.
 */
std::uint64_t RegField::getSize() const
{
    return size;
}

/*!
 * \brief Get the field's offset with respect to its parent field.
 *
 * \return Field's offset in number of bits (i.e. index of field's most significant bit in the referenced parent).
 */
std::uint64_t RegField::getOffset() const
{
    return offs;
}

/*!
 * \brief Get the field's total offset with respect to the whole register.
 *
 * \return Field's most significant bit index as seen from the register's top level.
 */
std::uint64_t RegField::getTotalOffset() const
{
    return parentTotalOffs - (parentSize-1) + offs;
}

//Private

/*!
 * \brief Set references to the immediate child fields.
 *
 * Makes the field aware of its immediate child fields and thereby makes their proxy references
 * accessible from the field via RegField::operator[](std::string_view) const.
 *
 * \param pChildFields Map of child field proxy references with their field names as keys.
 */
void RegField::setChildFields(std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> pChildFields)
{
    childFields.swap(pChildFields);
    repetitionKeys.clear();     //Should not be needed in practice, just to make really sure this was not set before by the other overload
}

/*!
 * \brief Assign field repetition numbers to actual child field names.
 *
 * Makes the field aware of its repetitions (in the form of immediate child fields) and thereby makes their
 * proxy references accessible from the field via RegField::n() (and RegField::operator[](std::string_view) const).
 *
 * \param pFieldReps Vector of field repetition proxy references (as pairs together with their assigned field "names").
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
