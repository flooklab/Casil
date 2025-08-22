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

#include <algorithm>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>

namespace
{

using RegField = casil::Layers::RL::StandardRegister::RegField;
using BoolRef = casil::Layers::RL::StandardRegister::BoolRef;

/*
 * Creates and returns a 'pSize' long vector of proxy class instances for bit manipulation of a contiguous
 * range of 'pSize' bits that are stored in 'pBits' at a most significant bit offset of 'pOffs'.
 * The indexing is hence such that returnValue[(pSize-1):0] ^= pBits[pOffs:(pOffs-(pSize-1))].
 *
 * Boundaries are not checked here but used BoolRef::BoolRef() will throw std::invalid_argument if an index exceeds the referenced bitset.
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
 *
 * Boundaries are not checked here but used BoolRef::BoolRef() will throw std::invalid_argument if an index exceeds the referenced field.
 */
std::vector<BoolRef> createBitRefs(const RegField& pParent, const std::uint64_t pSize, const std::uint64_t pOffs)
{
    std::vector<BoolRef> retVal;
    retVal.reserve(pSize);
    for (std::size_t i = pOffs-(pSize-1); i <= pOffs; ++i)
        retVal.emplace_back(pParent, i);
    return retVal;
}

/*
 * Creates and returns a 'pSize' long vector of proxy class instances for bit manipulation of a contiguous range of
 * 'pSize' bits from (and in turn indirectly referenced by) 'pParent' at a most significant bit offset of 'pOffs'.
 * The actual bit indices are permuted according to 'pBitOrder', which must be a 'pSize' long vector with unique indices between
 * 0 and 'pSize'-1. The indexing is hence such that returnValue[pBitOrder[0:(pSize-1)]] ^= pParent[pOffs:(pOffs-(pSize-1))],
 * which for regular bit ordering with pBitOrder = [(pSize-1), ..., 0] would simplify to
 * returnValue[(pSize-1):0] ^= pParent[pOffs:(pOffs-(pSize-1))] just as in
 * createBitRefs(const RegField&, std::uint64_t, std::uint64_t) above.
 *
 * Boundaries and bit order conditions are not checked here but the used BoolRef::BoolRef()
 * will at least throw std::invalid_argument if an index exceeds the referenced field.
 */
std::vector<BoolRef> createBitRefs(const RegField& pParent, const std::uint64_t pSize, const std::uint64_t pOffs,
                                   const std::vector<std::uint64_t>& pBitOrder)
{
    std::vector<std::size_t> idxs;
    idxs.reserve(pSize);
    for (std::size_t i = pOffs-(pSize-1); i <= pOffs; ++i)
        idxs.push_back(i);

    std::vector<BoolRef> retVal;
    retVal.reserve(pSize);
    for (std::size_t i = 0; i < pSize; ++i)
        retVal.emplace_back(pParent, idxs[pBitOrder[(pSize-1)-i]]);

    return retVal;
}

/*
 * Creates and returns a vector of proxy class instances for bit manipulation of an arbitrarily
 * ordered set of bits (with indices 'pIdxs') from (and in turn indirectly referenced by) 'pParent'.
 * The indexing is such that returnValue[(pIdxs.size()-1):0] ^= pParent[pIdxs[0]:pIdxs[pIdxs.size()-1]].
 *
 * Boundaries are not checked here but used RegField::operator[](std::size_t)
 * will throw std::invalid_argument if an index exceeds the referenced field.
 */
std::vector<BoolRef> createBitRefs(const RegField& pParent, const std::vector<std::size_t>& pIdxs)
{
    std::vector<BoolRef> retVal;
    retVal.reserve(pIdxs.size());
    for (auto idx : (pIdxs | std::views::reverse))
        retVal.push_back(pParent[idx]);                                                                 // cppcheck-suppress useStlAlgorithm
    return retVal;
}

/*
 * This is a helper function for RegField::RegField().
 *
 * Checks if 'pSize' is larger than zero and throws std::invalid_argument otherwise.
 *
 * Returns 'pSize'.
 */
std::uint64_t checkFieldSize(const std::uint64_t pSize)
{
    if (pSize == 0)
        throw std::invalid_argument("Invalid field size (must be larger than zero).");

    return pSize;
}

/*
 * This is a helper function for RegField::RegField().
 *
 * Checks if the register field specified by values 'pSize' and 'pOffs' lies within the extent
 * of its parent field with size 'pParentSize' and throws std::invalid_argument otherwise.
 *
 * Returns 'pOffs'.
 */
std::uint64_t checkFieldOffset(const std::uint64_t pOffs, const std::uint64_t pSize, const std::uint64_t pParentSize)
{
    if ((pSize > pOffs+1) || (pOffs >= pParentSize))
        throw std::invalid_argument("Field exceeds parent field's extent.");

    return pOffs;
}

/*
 * This is a helper function for RegField::RegField() (and StandardRegister::populateFieldTree()).
 *
 * Checks if the bit order permutation 'pBitOrder' has 'pSize' index elements with each index having a
 * value between 0 and 'pSize'-1 and if there are no duplicates (which overall implies that 'pBitOrder'
 * contains every index from 0 to 'pSize'-1) and throws std::invalid_argument otherwise.
 *
 * Returns 'pBitOrder'.
 */
std::vector<std::uint64_t> checkBitOrder(const std::vector<std::uint64_t>& pBitOrder, const std::uint64_t pSize)
{
    if (pBitOrder.size() != pSize)
        throw std::invalid_argument("Bit order sequence length differs from field size.");

    const std::set<std::uint64_t> tSet(pBitOrder.begin(), pBitOrder.end());
    if (tSet.size() != pSize)
        throw std::invalid_argument("Bit order sequence contains duplicate bit numbers.");

    for (const auto num : pBitOrder)
        if (num >= pSize)                                                                               // cppcheck-suppress useStlAlgorithm
            throw std::invalid_argument("Bit number exceeds field's extent.");

    return pBitOrder;
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
 * Also gets the optional "auto_start" value from \p pConfig (boolean value, default: false), which
 * defines whether write() shall automatically call Driver::exec() (see write() for more information).
 *
 * Also gets the optional "lsb_side_padding" value from \p pConfig (boolean value, default: true), which defines
 * whether bits in byte sequences (as used in write(), read(), toBytes() and fromBytes()) shall be "left-aligned",
 * meaning that the respective \e most significant byte (first byte of vector, index 0) always immediately starts
 * with the most significant bit of the corresponding bit sequence and possible zero-padding bits (for complete bytes)
 * are inserted on the side of the \e least significant bit/byte. If "lsb_side_padding" is set to false, possible
 * zero-padding bits will be inserted on the side of the \e most significant bit/byte. For some reason the former
 * is the fixed behavior in basil but can be configured here to facilitate working with drivers such as HL::GPIO.
 *
 * Any field in the abovementioned field configuration sequence is a map consisting of at least a field name (key "name"),
 * bit offset (key "offset") and bit size (key "size"). The offset denotes the index that the field's most significant bit has
 * in the parent field (assuming the most significant bit always has the highest index, i.e. the register and each of its fields
 * are ordered with least significant bit first). The field name must not begin with '#' and must not contain any periods ('.').
 *
 * Every field can optionally define its own nested field sequence (key "fields") to further define sub-fields,
 * which all have to be fully contained within the defining field.
 *
 * If the unsigned integer value "repeat" is defined for a field and larger than 1, an intermediate layer (between the field and
 * possibly sub-fields) of sub-fields is automatically inserted, representing identical (except bit shifted) repetitions/copies of
 * the given field configuration including its sub-fields. The actual size of the field then becomes \c repeat \c * \c size, while
 * each of the repetitions has a size of \c size and has an accordingly shifted offset with respect to the full repeated field.
 * The n-th repetition can be accessed via RegField::n() or the regular RegField::operator[](std::string_view) const /
 * StandardRegister::operator[](const std::string&) const using "#0", "#1", etc. as keys.
 * Note that here the zeroth repetition is the one starting at the highest offset (as inherited from basil).
 *
 * If the unsigned integer sequence "bit_order" is defined for a field, the order in which the parent field's bits get
 * referenced is changed according to the permutation that is specified by this sequence. This does not change the field's
 * properties (such as its offset) otherwise nor which bit range of the parent field gets referenced, just the internal
 * mapping/order of which field bit points to which of the parent field's bits. The regular order (most significant bit
 * points to most significant bit) is equivalent to a descending bit order sequence as <tt>[size-1, size-2, ..., 0]</tt>.
 * It is important to note that this implementation is different from the one in basil (which only applies the order on assignments),
 * because the symmetric solution seems to make much more sense and the feature seems to have been rarely used anyway.
 *
 * The bit order sequence must have the same size as the field and contain every index out of <tt>[0, size)</tt>,
 * which implies that duplicate bit assignments are not possible. Note that if the field gets repeated (see above) the
 * specified bit order applies to each field repetition independently. Setting a bit order for the \e whole field is then not
 * possible. Also note that child fields access their parent field's bits via RegField::operator[](std::size_t pIdx) const,
 * i.e. the parent's bit order is transparent and implicitly applies to its childs. For more details on the bit order see
 * also RegField(const RegField&, const std::string&, std::uint64_t, std::uint64_t, const std::vector<std::uint64_t>&).
 *
 * An optional "init" map can be specified in \p pConfig in order to define default values for specific register fields.
 * The keys in that map define the fields by their paths (as used in the argument to operator[](const std::string&) const).
 * The values can be either unsigned integers (as used in the argument to RegField::operator=(std::uint64_t) const) or strings
 * of zeros and ones with "0b" prefix for directly constructing the underlying bit sequences (represented bit sequence as used
 * in the argument to RegField::operator=(const boost::dynamic_bitset<>&) const). As an example, the format of the init map
 * should be equivalent to <tt>init: {"Path.To.Reg.Field.1": 1234, "Path.To.Another.Field": "0b1011001"}</tt>.
 *
 * The fields that appear in the init map will be automatically set to the specified values at initialization of the component
 * (see init()) and can be manually set to their init values again by calling applyDefaults(). Fields without such an init value
 * stay unchanged and init map entries that point to no existing field will be ignored. Setting the values happens as if by calling
 * RegField::operator=(std::uint64_t) const / RegField::operator=(const boost::dynamic_bitset<>&) const. Note that in case of the
 * bit sequence the sequence/string length must be equal to the field length. Also note that there must not be multiple/conflicting
 * assignments for overlapping/nested fields as no guarantee is made about the order in which the fields will be set.
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
 * \throws std::runtime_error If parsing of one of the integer values ("size", "offset", "repeat") fails.
 * \throws std::runtime_error If parsing of a "bit_order" sequence fails (no sequence, invalid format).
 * \throws std::runtime_error If a field size is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field repetition count is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field's bit order is invalid (wrong length, duplicate indices, index out of range).
 * \throws std::runtime_error If a field name is invalid (contains '.' or starts with '#').
 * \throws std::runtime_error If the same field name is defined multiple times within a group of child fields.
 * \throws std::runtime_error If a field exceeds the parent field's extent.
 * \throws std::runtime_error If an init value or init bit sequence from \p pConfig has an invalid format for one of the register fields.
 * \throws std::runtime_error If the length of an init bit sequence from \p pConfig (string length excluding prefix)
 *                            does not match the field size for one of the register fields.
 *
 * \param pName Component instance name.
 * \param pDriver %Driver instance to be used.
 * \param pConfig Component configuration.
 */
StandardRegister::StandardRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig) :
    Register(typeName, std::move(pName), pDriver, std::move(pConfig), LayerConfig::fromYAML("{size: uint}")),
    size(config.getUInt("size", 0)),
    autoStart(config.getBool("auto_start", false)),
    lsbSidePadding(config.getBool("lsb_side_padding", true)),
    data(size, 0),
    readData(size, 0),
    fields(),
    readFields(),
    initValues()
{
    if (size == 0)
        throw std::runtime_error("Invalid register size set for " + getSelfDescription() + ".");

    //Fill property tree with register field hierarchy, starting with unnamed root field
    fields.data() = std::make_shared<RegField>(data, "", size, size-1);
    const boost::property_tree::ptree fieldsConfig = config.getRawTreeAt("fields");
    populateFieldTree(fields, fieldsConfig, "fields");

    //Also fill field tree for driver readback data
    readFields.data() = std::make_shared<RegField>(readData, "", size, size-1);
    populateFieldTree(readFields, fieldsConfig, "fields");

    //Initially fill init value map for every register field with no value (i.e. std::monostate) set

    std::function<void(std::map<std::string, VariantValueType>&, const FieldTree&, const std::string&)> fillInitValues =
            [&fillInitValues](std::map<std::string, VariantValueType>& pInitValues, const FieldTree& pFieldTree,
                              const std::string& pParentKey) -> void
    {
        if (pParentKey != "")
            pInitValues[pParentKey] = std::monostate{};

        for (const auto& [key, subTree] : pFieldTree)
        {
            if (pParentKey == "")
                fillInitValues(pInitValues, subTree, key);
            else
                fillInitValues(pInitValues, subTree, pParentKey + "." + key);
        }
    };

    fillInitValues(initValues, fields, "");

    //Collect default values from "init" map of configuration YAML

    for (auto& [fieldPath, value] : initValues)
    {
        if (config.contains(LayerConfig::fromYAML("{init: {" + fieldPath + ": uint}}"), true))
        {
            value = config.getUInt("init." + fieldPath);
        }
        else if (config.contains(LayerConfig::fromYAML("{init: {" + fieldPath + ": }}"), false))
        {
            const std::string bitSeqStr = config.getStr("init." + fieldPath);

            if (bitSeqStr == "")    //(Need to) ignore empty nodes as most likely just intermediate nodes not actually set in the init map
                continue;

            if (!bitSeqStr.starts_with("0b"))
            {
                throw std::runtime_error("Invalid init bit sequence for register field \"" + fieldPath + "\" "
                                         "of  standard register \"" + name + "\".");
            }

            const std::string bitStr = bitSeqStr.substr(2);

            //Assume bit sequence, which must consist of '0's and '1's
            if (std::any_of(bitStr.begin(), bitStr.end(), [](const char c){ return (c != '0' && c != '1'); }))
            {
                throw std::runtime_error("Invalid init bit sequence for register field \"" + fieldPath + "\" "
                                         "of  standard register \"" + name + "\".");
            }

            const RegField& field = *(fields.get_child(fieldPath).data());
            if (bitStr.size() != field.getSize())
            {
                throw std::runtime_error("Init bit sequence for register field \"" + fieldPath + "\" of standard register "
                                         "\"" + name + "\" has wrong size.");
            }

            value = boost::dynamic_bitset(std::string(bitStr));
        }
    }
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

/*!
 * \brief Get the root field node for driver readback data.
 *
 * \return The top-level register field for readback data, which represents the \e entire readback data.
 */
const StandardRegister::RegField& StandardRegister::rootRead() const
{
    return *(readFields.data());
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

//

/*!
 * \brief Set register fields to configured default/init values.
 *
 * Every register field that has a default value/sequence defined in the "init" map in the
 * component configuration (see StandardRegister()) will be assigned to this init value/sequence.
 *
 * Note: The order of assignments might be arbitrary.
 */
void StandardRegister::applyDefaults()
{
    for (auto& [fieldPath, value] : initValues)
    {
        if (std::holds_alternative<std::uint64_t>(value))
            operator[](fieldPath) = std::get<std::uint64_t>(value);
        else if (std::holds_alternative<boost::dynamic_bitset<>>(value))
            operator[](fieldPath) = std::get<boost::dynamic_bitset<>>(value);
    }
}

//

/*!
 * \brief Assign equivalent integer value to the register.
 *
 * Assigns the binary equivalent of \p pValue to the register bits, in the same way as
 * assigning to the root() field via RegField::operator=(std::uint64_t) const (see there).
 *
 * \param pValue Value to be assigned.
 */
void StandardRegister::set(const std::uint64_t pValue) const
{
    root() = pValue;
}

/*!
 * \brief Assign a raw bit sequence to the register.
 *
 * Assigns \p pBits to the register bits, in the same way as assigning to the root()
 * field via RegField::operator=(const boost::dynamic_bitset<>&) const (see there).
 *
 * \throws std::invalid_argument If RegField::operator=(const boost::dynamic_bitset<>&) const throws \c std::invalid_argument
 *                               (i.e. if the size of \p pBits differs from the register size).
 *
 * \param pBits Bit sequence to be assigned.
 */
void StandardRegister::set(const boost::dynamic_bitset<>& pBits) const
{
    root() = pBits;
}

/*!
 * \brief Set/unset all register bits at once.
 *
 * Assigns \p pValue to every register bit (same as calling RegField::setAll() for the root() field).
 *
 * \param pValue True for bits high (1) and false for bits low (0).
 */
void StandardRegister::setAll(const bool pValue) const
{
    root().setAll(pValue);
}

//

/*!
 * \brief Get the register data as raw bit sequence.
 *
 * Returns the current state of the register content as bit sequence.
 *
 * \return <tt>register[(size-1):0]</tt>.
 */
boost::dynamic_bitset<> StandardRegister::get() const
{
    return data;
}

/*!
 * \brief Get the driver readback data as a bit sequence.
 *
 * Returns the current state of the driver readback data (as set by read()) as bit sequence.
 *
 * \return <tt>readback[(size-1):0]</tt>.
 */
boost::dynamic_bitset<> StandardRegister::getRead() const
{
    return readData;
}

//

/*!
 * \brief Write the register data to the driver.
 *
 * Calls Driver::setData() on the register's driver instance with the register data passed as argument (in form
 * of a byte sequence as in toBytes(), possibly truncated to a smaller length \p pNumBytes). If the "auto_start"
 * setting was enabled in the component configuration (see StandardRegister()), calls Driver::exec() afterwards.
 *
 * \throws std::invalid_argument If \p pNumBytes exceeds the register byte size (full byte count occupied by all register bits).
 *
 * \param pNumBytes Number of bytes of the byte sequence to actually use, or zero to use the full length.
 */
void StandardRegister::write(std::size_t pNumBytes) const
{
    if (size % 8 > 0)
    {
        if (pNumBytes > (size / 8) + 1)
            throw std::invalid_argument("Number of bytes exceeds register byte size for " + getSelfDescription() + ".");
        else if (pNumBytes == 0)
            pNumBytes = (size / 8) + 1;
    }
    else
    {
        if (pNumBytes > size / 8)
            throw std::invalid_argument("Number of bytes exceeds register byte size for " + getSelfDescription() + ".");
        else if (pNumBytes == 0)
            pNumBytes = size / 8;
    }

    if (lsbSidePadding && (size % 8) > 0)   //Need to add LSB-side padding bits (as is done in basil)
    {
        const std::uint64_t numPadBits = 8 - size % 8;
        boost::dynamic_bitset<> paddedData = data;
        paddedData.resize(size + numPadBits);
        paddedData <<= numPadBits;
        std::vector<std::uint8_t> tBytes = Bytes::bytesFromBitset(paddedData, (size / 8) + 1);
        tBytes.resize(pNumBytes);
        driver.setData(tBytes);
    }
    else
    {
        std::vector<std::uint8_t> tBytes = Bytes::bytesFromBitset(data, ((size - 1) / 8) + 1);
        tBytes.resize(pNumBytes);
        driver.setData(tBytes);
    }

    if (autoStart)
        driver.exec();
}

/*!
 * \brief Read from the driver and assign to the readback data.
 *
 * Calls Driver::getData() on the register's driver instance with \p pNumBytes as argument in order to get a byte sequence
 * of length \p pNumBytes (automatically set to <tt>((regBitSize - 1) / 8) + 1</tt> if zero). If \p pNumBytes is smaller
 * than <tt>((regBitSize - 1) / 8) + 1</tt>, zeros will be appended to that byte sequence to obtain a length of
 * <tt>((regBitSize - 1) / 8) + 1</tt>. The driver readback data will then be set to the bit sequence that is
 * represented by the byte sequence (as is done in fromBytes() for the regular register data).
 *
 * \throws std::invalid_argument If \p pNumBytes exceeds the register byte size (full byte count occupied by all register bits).
 * \throws std::runtime_error If the driver instance returns the wrong number of bytes (see \p pNumBytes).
 *
 * \param pNumBytes Number of bytes to actually get from the driver, or zero to get bytes for the full register size.
 */
void StandardRegister::read(std::size_t pNumBytes)
{
    if (size % 8 > 0)
    {
        if (pNumBytes > (size / 8) + 1)
            throw std::invalid_argument("Number of bytes exceeds register byte size for " + getSelfDescription() + ".");
        else if (pNumBytes == 0)
            pNumBytes = (size / 8) + 1;
    }
    else
    {
        if (pNumBytes > size / 8)
            throw std::invalid_argument("Number of bytes exceeds register byte size for " + getSelfDescription() + ".");
        else if (pNumBytes == 0)
            pNumBytes = size / 8;
    }

    std::vector<std::uint8_t> rawData = driver.getData(pNumBytes);

    if (rawData.size() != pNumBytes)
        throw std::runtime_error("Driver returned wrong number of bytes for " + getSelfDescription() + ".");

    rawData.resize(((size - 1) / 8) + 1);

    boost::dynamic_bitset<> tBits = Bytes::bitsetFromBytes(rawData, rawData.size() * 8);

    if (lsbSidePadding)     //Expect raw data bytes to be left-aligned (i.e. with LSB-side zero padding, according to component configuration)
        tBits >>= tBits.size() - size;

    tBits.resize(size);

    readData = tBits;
}

//

/*!
 * \brief Convert the register data to a byte sequence.
 *
 * Converts the sequence of register bits into a byte sequence in big endian byte order (i.e. least significant register
 * \c bit[0] ends up in <tt>byteSeq[byteSize-1]</tt>, which is the least significant byte). The bit alignment is made such that
 * zero-padding bits are either inserted at the front of the most significant byte or at the back of the least significant byte,
 * depending on the "lsb_side_padding" setting from the component configuration (see StandardRegister()).
 *
 * See also Bytes::bytesFromBitset().
 *
 * \return Register bits as (aligned) byte sequence.
 */
std::vector<std::uint8_t> StandardRegister::toBytes() const
{
    if (size % 8 > 0)
    {
        if (lsbSidePadding)     //Need to add LSB-side padding bits (as is done in basil)
        {
            const std::uint64_t numPadBits = 8 - size % 8;
            boost::dynamic_bitset<> paddedData = data;
            paddedData.resize(size + numPadBits);
            paddedData <<= numPadBits;
            return Bytes::bytesFromBitset(paddedData, (size / 8) + 1);
        }
        else
            return Bytes::bytesFromBitset(data, (size / 8) + 1);
    }
    else
        return Bytes::bytesFromBitset(data, size / 8);
}

/*!
 * \brief Load/assign the register data from a byte sequence.
 *
 * Sets the register content to the bit sequence equivalent of \p pBytes in terms of the inverse operation to toBytes().
 * This means that \p pBytes must be a byte sequence representation of the wanted register bit sequence in in big endian
 * byte order (i.e. least significant register \c bit[0] is in <tt>byteSeq[byteSize-1]</tt>, which is the least significant byte).
 * The bit alignment must be such that zero-padding bits are either at the front of the most significant byte or at the back of the
 * least significant byte, depending on the "lsb_side_padding" setting from the component configuration (see StandardRegister()).
 *
 * See also Bytes::bitsetFromBytes().
 *
 * \throws std::invalid_argument If length of \p pBytes differs from the register byte size (full byte count occupied by all register bits).
 *
 * \param pBytes Desired register bits as (aligned) byte sequence.
 */
void StandardRegister::fromBytes(const std::vector<std::uint8_t> pBytes)
{
    if (pBytes.size() != ((size - 1) / 8) + 1)
        throw std::invalid_argument("Byte sequence length differs from register byte size for " + getSelfDescription() + ".");

    boost::dynamic_bitset<> tBits = Bytes::bitsetFromBytes(pBytes, pBytes.size() * 8);

    if (lsbSidePadding)     //Expect raw data bytes to be left-aligned (i.e. with LSB-side zero padding, according to component configuration)
        tBits >>= tBits.size() - size;

    tBits.resize(size);

    data = tBits;
}

//Private

/*!
 * \copybrief Register::initImpl()
 *
 * Sets the register fields to possibly configured default/init values, see applyDefaults().
 *
 * \return True.
 */
bool StandardRegister::initImpl()
{
    applyDefaults();

    return true;
}

/*!
 * \copybrief Register::closeImpl()
 *
 * \return True.
 */
bool StandardRegister::closeImpl()
{
    return true;
}

//

/*!
 * \copybrief Register::loadRuntimeConfImpl()
 *
 * Sets the register content (whole register and/or individual fields) according to the values specified in \p pConf.
 *
 * \p pConf is expected to be an ordered list (at the top level) of register/field to value assignments where
 * each of the list nodes can be either a node with just a value (to assign to the whole register) or a node
 * with just one child node (to assign to a specific register field), which can in turn have another single
 * child node or a value (and so forth). The child node keys determine the path of the to be assigned register field.
 * Every value must be a string of zeros and ones with "0b" prefix as direct representation of the referred bit sequence.
 *
 * To provide an example, \p pConf might look similar to this:
 * - \c pConf: no %data() at node
 *  - #0: "0b101010101010"
 *  - #1: no %data() at node
 *   - Some: no %data() at node
 *    - Field: no %data() at node
 *     - Path: "0b111"
 *  - #2: no %data() at node
 *   - AnotherField: "0b1010"
 *
 * Note that this can be optimally achieved for LayerBase::loadRuntimeConfiguration() by using a YAML sequence,
 * such as <tt>["0b101010101010", {Some.Field.Path: "0b111"}, {AnotherField: "0b1010"}]</tt>.
 *
 * The register/field assignments will be made in the specified order and using RegField::operator=(const boost::dynamic_bitset<>&) const.
 *
 * Note: The length of every bit sequence must exactly match the register/field size.
 *
 * \throws std::runtime_error If a node in \p pConf has neither non-empty data nor a child node.
 * \throws std::runtime_error If a node in \p pConf has \e both non-empty data and a child node.
 * \throws std::runtime_error If a node in \p pConf has multiple child nodes.
 * \throws std::runtime_error If a bit sequence string has invalid format (missing prefix, invalid characters).
 * \throws std::runtime_error If the bit sequence length differs from the register or register field size.
 * \throws std::runtime_error If a register field does not exist.
 *
 * \param pConf Desired runtime configuration tree.
 * \return If successful.
 */
bool StandardRegister::loadRuntimeConfImpl(boost::property_tree::ptree&& pConf)
{
    /*
     * Converts string ('pBitStr') of zeros and ones with "0b" prefix to a corresponding bitset of
     * matching length and returns that. Throws std::invalid_argument if number of bits differs from
     * 'pSize', if 'pBitStr' does not start with the prefix or if it contains invalid characters.
     */
    auto parseBitStr = [](const std::string& pBitStr, const std::uint64_t pSize) -> boost::dynamic_bitset<>
    {
        if (!pBitStr.starts_with("0b"))
            throw std::invalid_argument("Invalid bit sequence string.");

        const std::string bitSeqStr = pBitStr.substr(2);

        //Bit sequence string must consist of '0's and '1's
        if (std::any_of(bitSeqStr.begin(), bitSeqStr.end(), [](const char c){ return (c != '0' && c != '1'); }))
            throw std::invalid_argument("Invalid characters in bit sequence string.");

        if (bitSeqStr.size() != pSize)
            throw std::invalid_argument("Bit sequence length does not match register size.");

        return boost::dynamic_bitset(bitSeqStr);
    };

    /*
     * Recursively traverses 'pConfTree' in order to find the eventual branch tip and then sets 'pBitStr'
     * to its value and 'pPath' to its path. Each node of 'pConfTree' must have either exactly
     * one child node or a non-empty value/data. std::runtime_error is thrown otherwise.
     */
    std::function<void(const boost::property_tree::ptree&, std::string&, std::string&)> recurseConfTree =
            [&recurseConfTree](const boost::property_tree::ptree& pConfTree, std::string& pPath, std::string& pBitStr) -> void
    {
        if (pConfTree.empty() && pConfTree.data() == "")
            throw std::runtime_error("Node has neither non-empty data nor a child node.");
        else if (!pConfTree.empty() && pConfTree.data() != "")
            throw std::runtime_error("Node must have either non-empty data or a child node.");
        else if (pConfTree.empty())
        {
            pBitStr = pConfTree.data();
            return;
        }
        else if (pConfTree.size() == 1)
        {
            const auto [key, subTree] = pConfTree.front();

            if (pPath == "")
                pPath.append(key);
            else
                pPath.append("." + key);

            recurseConfTree(subTree, pPath, pBitStr);
        }
        else
            throw std::runtime_error("Node has multiple child nodes.");
    };

    for (const auto& [key, subTree] : pConf)
    {
        if (subTree.empty() && subTree.data() == "")
            throw std::runtime_error("Node has neither non-empty data nor a child node.");
        else if (!subTree.empty() && subTree.data() != "")
            throw std::runtime_error("Node must have either non-empty data or a child node.");
        else if (subTree.empty())
        {
            try
            {
                root() = parseBitStr(subTree.data(), size);
            }
            catch (const std::invalid_argument& exc)
            {
                throw std::runtime_error(exc.what());
            }
        }
        else if (subTree.size() == 1)
        {
            std::string fieldPath;
            std::string bitStr;
            recurseConfTree(subTree, fieldPath, bitStr);
            try
            {
                const RegField& field = this->operator[](fieldPath);
                field = parseBitStr(bitStr, field.getSize());
            }
            catch (const std::invalid_argument& exc)
            {
                throw std::runtime_error(exc.what());
            }
        }
        else
            throw std::runtime_error("Node has multiple child nodes.");
    }

    return true;
}

/*!
 * \copybrief Register::dumpRuntimeConfImpl()
 *
 * Takes the current register content as bit sequence (see e.g. get()), converts this to a string, adds a "0b" prefix and
 * constructs a "configuration tree" with a single child (at key "#0") that has its data set to this very "bit sequence string".
 *
 * Note: According to Auxil::propertyTreeFromYAML() the returned tree is equivalent
 * to a YAML sequence with a single element, such as <tt>["0b101010101010"]</tt>.
 * See also the example in loadRuntimeConfImpl().
 *
 * \return Current runtime configuration tree.
 */
boost::property_tree::ptree StandardRegister::dumpRuntimeConfImpl() const
{
    std::string bitStr;
    boost::to_string(root().toBits(), bitStr);

    boost::property_tree::ptree subTree;
    subTree.data() = "0b" + bitStr;

    boost::property_tree::ptree confTree;
    confTree.push_back({"#0", subTree});

    return confTree;
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
 * As can be seen, the "repeat" and "bit_order" entries are optional. Note that "bit_order" is actually supposed to be a \e sequence,
 * hence if a map \e is used (as in the example above) the key names have no effect because only the order matters (most significant
 * field bit index first). For more information on \p pConfTree, allowed values and their effects please refer to StandardRegister().
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
 * \throws std::runtime_error If parsing of one of the integer values ("size", "offset", "repeat") fails.
 * \throws std::runtime_error If parsing of a "bit_order" sequence fails (no sequence, invalid format).
 * \throws std::runtime_error If a field size is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field repetition count is set to zero or the corresponding unsigned integer conversion fails.
 * \throws std::runtime_error If a field's bit order is invalid (wrong length, duplicate indices, index out of range).
 * \throws std::runtime_error If a field name is invalid (contains '.' or starts with '#').
 * \throws std::runtime_error If the same field name is defined multiple times within a group of child fields.
 * \throws std::runtime_error If a field exceeds the parent field's extent.
 * \throws std::runtime_error Possibly/effectively if \p pParentKey is wrong (might cause one of the other exceptions).
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

        //Handle unknown field description elements
        static constexpr std::array<const char*, 6> knownFieldElems = {"name", "size", "offset", "fields", "repeat", "bit_order"};
        for (const auto& [fieldKey, fieldVal] : field)
        {
            (void)fieldVal;
            if (std::find(knownFieldElems.begin(), knownFieldElems.end(), fieldKey) == knownFieldElems.end())
                throw std::runtime_error("Unknown key \"" + fieldKey + "\" in register field definition for " + getSelfDescription() + ".");
        }

        //Handle missing but essential field description elements
        if (field.find("name") == field.not_found() || field.find("size") == field.not_found() ||
            field.find("offset") == field.not_found())
        {
            throw std::runtime_error("Missing key in register field definition for " + getSelfDescription() + ".");
        }

        //Field repetition number defined? (optional)
        const bool repsDefined = (field.find("repeat") != field.not_found());

        //Field repetition number defined? (optional)
        const bool orderDefined = (field.find("bit_order") != field.not_found());

        //Handle missing/invalid data of field description elements
        if (!field.get_child("name").empty() || field.get_child("name").data() == "" ||
            !field.get_child("size").empty() || field.get_child("size").data() == "" ||
            !field.get_child("offset").empty() || field.get_child("offset").data() == "" ||
            (repsDefined && (!field.get_child("repeat").empty() || field.get_child("repeat").data() == "")) ||
            (orderDefined && (field.get_child("bit_order").empty() || field.get_child("bit_order").data() != "")))
        {
            throw std::runtime_error("Invalid register field configuration for " + getSelfDescription() + ".");
        }

        //Compose layer configuration path for current field
        const std::string fullKey = pParentKey + "." + key;

        //Parse field description elements

        const std::string tName = config.getStr(fullKey + ".name");
        const std::optional<std::uint64_t> tSizeOpt = config.getUIntOpt(fullKey + ".size");
        const std::optional<std::uint64_t> tOffsOpt = config.getUIntOpt(fullKey + ".offset");
        const std::optional<std::uint64_t> tRepsOpt = repsDefined ? config.getUIntOpt(fullKey + ".repeat") : 1;
        const std::optional<std::vector<std::uint64_t>> tOrderOpt = orderDefined ? config.getUIntSeqOpt(fullKey + ".bit_order") : std::nullopt;

        if (!tSizeOpt)
            throw std::runtime_error("Could not parse size value for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (!tOffsOpt)
            throw std::runtime_error("Could not parse offset value for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (!tRepsOpt)
            throw std::runtime_error("Could not parse repetition count for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (orderDefined && !tOrderOpt)
            throw std::runtime_error("Could not parse bit order for register field \"" + tName + "\" of " + getSelfDescription() + ".");

        const std::uint64_t tSize = tSizeOpt.value();
        const std::uint64_t tOffs = tOffsOpt.value();
        const std::uint64_t tReps = tRepsOpt.value();
        const std::vector<std::uint64_t> tOrder = orderDefined ? tOrderOpt.value() : std::vector<std::uint64_t>{};
                                                                                    //Passing empty vector to RegField() yields regular order

        //Handle invalid values

        if (tSize == 0)
            throw std::runtime_error("Zero size set for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (tReps == 0)
            throw std::runtime_error("Zero repetitions set for register field \"" + tName + "\" of " + getSelfDescription() + ".");

        if (orderDefined)
        {
            try
            {
                ::checkBitOrder(tOrder, tSize);
            }
            catch (const std::invalid_argument& exc)
            {
                throw std::runtime_error("Invalid bit order sequence for register field \"" + tName + "\" "
                                         "of " + getSelfDescription() + ": " + exc.what());
            }
        }

        if (tName.find('.') != tName.npos || tName.starts_with('#'))
            throw std::runtime_error("Invalid name set for register field \"" + tName + "\" of " + getSelfDescription() + ".");
        if (pFieldTree.find(tName) != pFieldTree.not_found())
            throw std::runtime_error("Field with name \"" + tName + "\" is defined multiple times for " + getSelfDescription() + ".");

        const RegField& parentField = *(pFieldTree.data());

        //Field must be fully contained within the parent field
        if ((tSize*tReps > tOffs+1) || (tOffs >= parentField.getSize()))
            throw std::runtime_error("Register field \"" + tName + "\" exceeds parent field's extent for " + getSelfDescription() + ".");

        //Create new field tree branch for current field
        FieldTree tSubTree;

        if (tReps > 1)  //Need to (iteratively) add nested layer of fields to reflect and enable access to individual repetitions
        {
            //Add proxy instance for entire current field (i.e. spanning all repetitions)
            tSubTree.data() = std::make_shared<RegField>(parentField, tName, tSize*tReps, tOffs);

            const RegField& parentFieldForReps = *(tSubTree.data());

            for (std::uint64_t i = 0; i < tReps; ++i)
            {
                const std::string tSubName = "#" + std::to_string(i);

                //Create new field tree branch for current field repetition
                FieldTree tSubSubTree;

                //Add proxy instance for current field repetition
                tSubSubTree.data() = std::make_shared<RegField>(parentFieldForReps, tSubName, tSize, tSize*(tReps-i)-1, tOrder);

                //Recurse, if any sub-fields specified
                if (field.find("fields") != field.not_found())
                    populateFieldTree(tSubSubTree, field.get_child("fields"), fullKey + ".fields");

                //Can make current field repetition make aware of its immediate childs after recursion
                std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> childFieldRefs;
                for (const auto& [subSubFieldName, subSubField] : tSubSubTree)
                    childFieldRefs.emplace(subSubFieldName, *(subSubField.data()));
                tSubSubTree.data()->setChildFields(std::move(childFieldRefs));

                //Add new field tree branch for current field repetition to branch of the entire field
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
            //Add proxy instance for current field
            tSubTree.data() = std::make_shared<RegField>(parentField, tName, tSize, tOffs, tOrder);

            //Recurse, if any sub-fields specified
            if (field.find("fields") != field.not_found())
                populateFieldTree(tSubTree, field.get_child("fields"), fullKey + ".fields");

            //Can make current field make aware of its immediate childs after recursion
            std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> childFieldRefs;
            for (const auto& [subFieldName, subField] : tSubTree)
                childFieldRefs.emplace(subFieldName, *(subField.data()));
            tSubTree.data()->setChildFields(std::move(childFieldRefs));
        }

        //Add new field tree branch for current field to parent/existing tree
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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
BoolRef::BoolRef(boost::dynamic_bitset<>& pBits, /*const */std::size_t pIdx) :
#else
BoolRef::BoolRef(boost::dynamic_bitset<>& pBits, const std::size_t pIdx) :
#endif
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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
BoolRef::BoolRef(const RegField& pParent, /*const */std::size_t pIdx) :
#else
BoolRef::BoolRef(const RegField& pParent, const std::size_t pIdx) :
#endif
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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
bool BoolRef::operator=(/*const */bool pValue) const
#else
bool BoolRef::operator=(const bool pValue) const
#endif
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
 * Binds the instance's register field access functions to the "artificial" field that corresponds
 * to the potentially incoherent/disconnected set of bits with indices \p pIdxs within \p pParent.
 * The \e first index in \p pIdxs results in the field's \e most significant bit, i.e.
 * <tt>field[(pIdxs.size()-1):0] = pParent[pIdxs[0]:pIdxs[pIdxs.size()-1]]</tt>.
 *
 * The indices in \p pIdxs must be \e unique and each of them must be within the extent of \p pParent.
 *
 * \note This constructor is primarily made for being used by operator[](const std::vector<std::size_t>&) const. See also there.
 *
 * \note Other than a regular RegField (as constructed by one of the public constructors
 *       RegField(boost::dynamic_bitset<>&, const std::string&, std::uint64_t, std::uint64_t)
 *       or RegField(const RegField&, const std::string&, std::uint64_t, std::uint64_t))
 *       the field offset (see getOffset() and getTotalOffset()) is set such that it
 *       equals the \e largest index in \p pIdxs, regardless of its position.
 *
 * \note Invalid \p pIdxs correctly result in \c std::invalid_argument exceptions but for this constructor with possibly confusing
 *       or at least not very helpful exception texts. Hence the passed \p pIdxs should simply \e always be correct (see above).
 *
 * \throws std::invalid_argument If \p pIdxs is empty.
 * \throws std::invalid_argument If one of \p pIdxs exceeds the extent of \p pParent.
 * \throws std::invalid_argument If \p pIdxs contains duplicate indices.
 *
 * \param pParent Parent register field that itself references the bits of the referenced bit sequence.
 * \param pIdxs Indices of the bits to be referenced from \p pParent.
 */
RegField::RegField(const RegField& pParent, const std::vector<std::size_t>& pIdxs) :
    name(""),
    size(::checkFieldSize(pIdxs.size())),
    offs(::checkFieldOffset(*std::max_element(pIdxs.begin(), pIdxs.end()), pIdxs.size(), pParent.getSize())),
    parentSize(pParent.getSize()),
    parentTotalOffs(pParent.getTotalOffset()),
    dataRefs(::createBitRefs(pParent, pIdxs)),
    childFields(),
    repetitionKeys()
{
    std::set<std::size_t> idxSet;
    for (auto idx : pIdxs)
    {
        if (idxSet.contains(idx))
            throw std::invalid_argument("Indices must be unique.");
        else
            idxSet.insert(idx);
    }
}

//

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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
RegField::RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, /*const */std::uint64_t pSize, /*const */std::uint64_t pOffs) :
#else
RegField::RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs) :
#endif
    name(pName),
    size(::checkFieldSize(pSize)),
    offs(::checkFieldOffset(pOffs, pSize, pBits.size())),
    parentSize(pBits.size()),
    parentTotalOffs(pBits.size()-1),
    dataRefs(::createBitRefs(pBits, pSize, pOffs)),
    childFields(),
    repetitionKeys()
{
}

/*!
 * \brief Constructor.
 *
 * Binds the instance's register field access functions to the field (with name \p pName) that corresponds to the sequence of bits
 * within \p pParent with size \p pSize and offset \p pOffs, with individual bits possibly permuted by \p pBitOrder if set, i.e.:
 * - <tt>field[(pSize-1):0] = pParent[pOffs:(pOffs-(pSize-1))]</tt>, for \p pBitOrder empty (default), or
 * - <tt>field[pBitOrder[0:(pSize-1)]] = pParent[pOffs:(pOffs-(pSize-1))]</tt>, for specific, non-empty \p pBitOrder.
 *
 * If set to a non-empty vector, \p pBitOrder must have a size of \p pSize and contain every index out of <tt>[0, pSize)</tt>,
 * which defines the field-local bit permutation. For example, \p pBitOrder = <tt>{(pSize-1), (pSize-2), ..., 0}</tt> yields a regular
 * bit order with most significant bit first (as if \p pBitOrder was empty) and \p pBitOrder = <tt>{0, ..., (pSize-2), (pSize-1)}</tt>
 * yields an inversed bit order. Note that the bit order does not change the handling of the field itself but only the actual order of
 * the referenced bits from the \p pParent. When value \c x is assigned to the field then reading its value gives you \c x again.
 * However, reading the contiguous bit section for the field from \p pParent will be affected by the bit order (and vice versa).
 * Also note that possible \e child fields reference the field's bits independently of the field's bit \e order (it's "transparent")
 * and thus a field's bit order affects the \e relationship between its parent and its childs (the parent's grandchilds) in the same way.
 *
 * \throws std::invalid_argument If \p pSize is zero.
 * \throws std::invalid_argument If the field would exceed the extent of \p pParent (given values of \p pSize and \p pOffs).
 * \throws std::invalid_argument If length of \p pBitOrder differs from \p pSize.
 * \throws std::invalid_argument If \p pBitOrder contains duplicate indices.
 * \throws std::invalid_argument If an index in \p pBitOrder exceeds the field's extent (given value of \p pSize).
 *
 * \param pParent Parent register field that itself references the bits of the referenced bit sequence.
 * \param pName Name of the referenced register field.
 * \param pSize Size of the referenced field in number of bits.
 * \param pOffs Index of the referenced field's most significant bit in \p pParent.
 * \param pBitOrder Order/permutation of the \p pSize field bits with respect to \p pParent, or empty for regular order.
 */
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
RegField::RegField(const RegField& pParent, const std::string& pName, /*const */std::uint64_t pSize, /*const */std::uint64_t pOffs,
                   const std::vector<std::uint64_t>& pBitOrder) :
#else
RegField::RegField(const RegField& pParent, const std::string& pName, const std::uint64_t pSize, const std::uint64_t pOffs,
                   const std::vector<std::uint64_t>& pBitOrder) :
#endif
    name(pName),
    size(::checkFieldSize(pSize)),
    offs(::checkFieldOffset(pOffs, pSize, pParent.getSize())),
    parentSize(pParent.getSize()),
    parentTotalOffs(pParent.getTotalOffset()),
    dataRefs(pBitOrder.empty() ? ::createBitRefs(pParent, pSize, pOffs) :
                                 ::createBitRefs(pParent, pSize, pOffs, ::checkBitOrder(pBitOrder, pSize))),
    childFields(),
    repetitionKeys()
{
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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
std::uint64_t RegField::operator=(/*const */std::uint64_t pValue) const
#else
std::uint64_t RegField::operator=(const std::uint64_t pValue) const
#endif
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
 * \brief Assign equivalent integer value to the field.
 *
 * See operator=(std::uint64_t) const.
 *
 * \param pValue Value to be assigned.
 */
void RegField::set(const std::uint64_t pValue) const
{
    *this = pValue;
}

/*!
 * \brief Assign a raw bit sequence to the field.
 *
 * See operator=(const boost::dynamic_bitset<>&) const.
 *
 * \throws std::invalid_argument If operator=(const boost::dynamic_bitset<>&) const throws \c std::invalid_argument.
 *
 * \param pBits Bit sequence to be assigned.
 */
void RegField::set(const boost::dynamic_bitset<>& pBits) const
{
    *this = pBits;
}

/*!
 * \brief Set/unset all field bits at once.
 *
 * Assigns \p pValue to every bit that is referenced by the field.
 *
 * \param pValue True for bits high (1) and false for bits low (0).
 */
void RegField::setAll(const bool pValue) const
{
    for (std::size_t i = 0; i < size; ++i)
        dataRefs[i] = pValue;
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
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
const RegField& RegField::operator[](/*const */std::string_view pFieldName) const
#else
const RegField& RegField::operator[](const std::string_view pFieldName) const
#endif
{
    const auto it = childFields.find(pFieldName);

    if (it == childFields.end())
        throw std::invalid_argument("Register field \"" + name + "\" has no sub-field \"" + std::string(pFieldName) + "\".");

    return it->second;
}

/*!
 * \brief Access a specific bit in the field.
 *
 * \throws std::invalid_argument If \p pIdx exceeds the field size.
 *
 * \param pIdx Field-local bit number, assuming least significant bit first.
 * \return Proxy class instance for register field's bit \p pIdx.
 */
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
const BoolRef& RegField::operator[](/*const */std::size_t pIdx) const
#else
const BoolRef& RegField::operator[](const std::size_t pIdx) const
#endif
{
    if (pIdx >= size)
        throw std::invalid_argument("Index " + std::to_string(pIdx) + " is out of range for register field \"" + name + "\".");

    return dataRefs[pIdx];
}

/*!
 * \brief Access a slice of bits in the field.
 *
 * Creates (and returns) a custom register field on demand for a contiguous
 * slice of bits between (and including) indices \p pMsbIdx and \p pLsbIdx.
 *
 * If \p pLsbIdx is larger than \p pMsbIdx, the slice will be reversed (least significant bit first).
 *
 * \note The returned field only \e references its parent field (i.e. the field this function is called on)
 *       and consequently all parents thereof and ultimately the containing StandardRegister.
 *       Hence take their scope into account when trying to access the data through it.
 *
 * \throws std::invalid_argument If \p pMsbIdx exceeds the field size.
 * \throws std::invalid_argument If \p pLsbIdx exceeds the field size.
 *
 * \param pMsbIdx Field-local bit number for the \e most significant bit of the selected slice.
 * \param pLsbIdx Field-local bit number for the \e least significant bit of the selected slice.
 * \return New proxy class instance for <tt>field[pMsbIdx:pLsbIdx]</tt>.
 */
RegField RegField::operator()(const std::size_t pMsbIdx, const std::size_t pLsbIdx) const
{
    //TODO this function should eventually become operator[] (need C++23 for multi-args there)

    if (pMsbIdx >= size)
    {
        throw std::invalid_argument("Most significant bit index " + std::to_string(pMsbIdx) +
                                    " is out of range for register field \"" + name + "\".");
    }
    if (pLsbIdx >= size)
    {
        throw std::invalid_argument("Least significant bit index " + std::to_string(pLsbIdx) +
                                    " is out of range for register field \"" + name + "\".");
    }

    if (pLsbIdx > pMsbIdx)  //Make reverse slice by applying inverse bit order
    {
        const std::uint64_t sliceSize = pLsbIdx-pMsbIdx+1;
        std::vector<std::uint64_t> reverseBitOrder(sliceSize, 0);
        std::generate_n(reverseBitOrder.begin(), sliceSize, [bitNum = 0]() mutable { return bitNum++; });
        return RegField(*this, "", sliceSize, pLsbIdx, reverseBitOrder);
    }
    else
        return RegField(*this, "", pMsbIdx-pLsbIdx+1, pMsbIdx);
}

/*!
 * \brief Access a set of unique bits in the field.
 *
 * Creates (and returns) a custom register field on demand for an arbitrary selection of unique bits \p pIdxs.
 *
 * \note The returned field only \e references its parent field (i.e. the field this function is called on)
 *       and consequently all parents thereof and ultimately the containing StandardRegister.
 *       Hence take their scope into account when trying to access the data through it.
 *
 * \throws std::invalid_argument If \p pIdxs is empty.
 * \throws std::invalid_argument If one of \p pIdxs exceeds the field's extent.
 * \throws std::invalid_argument If \p pIdxs contains duplicate indices.
 *
 * \param pIdxs Unique, field-local bit numbers to form a new sub-field (in specified order).
 * \return New proxy class instance for <tt>field[pIdxs[0]:pIdxs[pIdxs.size()-1]]</tt>.
 */
RegField RegField::operator[](const std::vector<std::size_t>& pIdxs) const
{
    if (pIdxs.empty())
        throw std::invalid_argument("Number of selected indices must be larger than zero.");

    std::set<std::size_t> idxSet;
    for (auto idx : pIdxs)
    {
        if (idx >= size)
            throw std::invalid_argument("Index " + std::to_string(idx) + " is out of range for register field \"" + name + "\".");

        if (idxSet.contains(idx))
            throw std::invalid_argument("Selected indices must be unique.");
        else
            idxSet.insert(idx);
    }

    return RegField(*this, pIdxs);
}

/*!
 * \brief Access a set of unique bits in the field.
 *
 * This function improves overload resolution if operator[](const std::vector<std::size_t>&) const is targeted.
 *
 * \copydetails operator[](const std::vector<std::size_t>&) const
 */
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
RegField RegField::operator[](/*const */std::initializer_list<std::size_t> pIdxs) const
#else
RegField RegField::operator[](const std::initializer_list<std::size_t> pIdxs) const
#endif
{
    return operator[](std::vector<std::size_t>(pIdxs));
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
 * Note: For the special case of \e field \e repetitions the followig overload must be used instead:
 * setChildFields(const std::vector<std::pair<std::string, const std::reference_wrapper<const RegField>>>&)
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
