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

#ifndef CASIL_LAYERS_RL_STANDARDREGISTER_H
#define CASIL_LAYERS_RL_STANDARDREGISTER_H

#include <casil/RL/register.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/HL/driver.h>

#include <boost/dynamic_bitset.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace casil
{

namespace Layers::RL
{

/*!
 * \brief Basic register implementation for accessing raw driver data through a structured register view.
 *
 * Provides high-level access/manipulation to/of the driver-specific special data that can
 * be exposed by certain drivers via MuxedDriver::setData() and MuxedDriver::getData().
 *
 * The data to be written can be set/prepared as a whole, in terms of "register fields"
 * (structured/logical subdivisions of the register) or by setting individual bits.
 * This data can then be "committed" to the driver by calling write() (which calls
 * MuxedDriver::setData() and optionally also MuxedDriver::exec()).
 *
 * Data to be read from the driver (see MuxedDriver::getData()) can instead be fetched
 * by calling read() and can then be accessed similar to the write data (see getRead()
 * and rootRead()). However, this "readback" data is not meant to be changed (read-only).
 *
 * See StandardRegister() for more detailed information.
 */
class StandardRegister final : public Register
{
public:
    class BoolRef;
    class RegField;

public:
    StandardRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig);  ///< Constructor.
    ~StandardRegister() override = default;                                         ///< Default destructor.
    //
    RegField& operator[](const std::string& pFieldPath);                            ///< Access a specific register field.
    const RegField& operator[](const std::string& pFieldPath) const;                ///< Access a specific register field.
    BoolRef& operator[](std::size_t pIdx);                                          ///< Access a specific bit in the register.
    const BoolRef& operator[](std::size_t pIdx) const;                              ///< Access a specific bit in the register.
    //
    RegField& root();                                                               ///< Get the root field node.
    const RegField& root() const;                                                   ///< Get the root field node.
    const RegField& rootRead() const;                                               ///< Get the root field node for driver readback data.
    //
    std::uint64_t getSize() const;                                                  ///< Get the size of the register.
    //
    void applyDefaults();                                                           ///< Set register fields to configured default/init values.
    //
    void set(std::uint64_t pValue);                                                 ///< Assign equivalent integer value to the register.
    void set(const boost::dynamic_bitset<>& pBits);                                 ///< Assign a raw bit sequence to the register.
    void setAll(bool pValue = true);                                                ///< Set/unset all register bits at once.
    //
    const boost::dynamic_bitset<>& get() const;                                     ///< Get the register data as raw bit sequence.
    const boost::dynamic_bitset<>& getRead() const;                                 ///< Get the driver readback data as a bit sequence.
    //
    void write(std::size_t pNumBytes = 0) const;                                    ///< Write the register data to the driver.
    void read(std::size_t pNumBytes = 0);                                           ///< Read from the driver and assign to the readback data.
    //
    std::vector<std::uint8_t> toBytes() const;                                      ///< Convert the register data to a byte sequence.
    void fromBytes(std::vector<std::uint8_t> pBytes);                               ///< Load/assign the register data from a byte sequence.

private:
    bool initImpl() override;
    bool closeImpl() override;
    //
    void loadRuntimeConfImpl(boost::property_tree::ptree&& pConf) override;
    boost::property_tree::ptree dumpRuntimeConfImpl() const override;
    //
    typedef boost::property_tree::basic_ptree<std::string, std::shared_ptr<RegField>> FieldTree;    ///< \brief Property tree for register
                                                                                                    ///  fields that reference parts of the
                                                                                                    ///  register with the field names as keys.
    void populateFieldTree(FieldTree& pFieldTree, const boost::property_tree::ptree& pConfTree, const std::string& pParentKey) const;
                                                                                                ///< \brief Populate the register field tree by
                                                                                                ///  recursing through the field configuration.

private:
    const std::uint64_t size;           ///< Size of the register in number of bits.
    //
    const bool autoStart;               ///< Automatically call Driver::exec() from within write().
    const bool lsbSidePadding;          ///< Use/expect left-alignment of byte vectors (i.e. with LSB-side zero padding!) as is done in basil.
    //
    boost::dynamic_bitset<> data;       ///< Register content (for writing).
    boost::dynamic_bitset<> readData;   ///< Driver readback data (like above register content but for reading).
    //
    FieldTree fields;                   ///< Tree representing the hierarchy of named register fields for convenient access to them.
    FieldTree readFields;               ///< Equivalent field tree that points to the driver readback data instead.
    //
    typedef std::variant<std::monostate, std::uint64_t, boost::dynamic_bitset<>> VariantValueType;
                                                            ///< Variant to optionally store the two possible register field assignment types.
    std::map<std::string, VariantValueType> initValues;     ///< Register fields' default values from YAML configuration.

    CASIL_REGISTER_REGISTER_H("StandardRegister")
};

/*!
 * \brief Proxy class for accessing an individual register bit.
 *
 * Provides convenient read/write access to a single bit of a StandardRegister instance or of a specific register field thereof.
 */
class StandardRegister::BoolRef                                 // cppcheck-suppress noConstructor symbolName=BoolRef
{
public:
    BoolRef(boost::dynamic_bitset<>& pBits, std::size_t pIdx);  ///< Constructor.
    BoolRef(RegField& pParent, std::size_t pIdx);               ///< Constructor.
    BoolRef(const BoolRef&) = default;                          ///< Default copy constructor.
    BoolRef(BoolRef&&) = delete;                                ///< Deleted move constructor.
    ~BoolRef() = default;                                       ///< Default destructor.
    //
    BoolRef& operator=(const BoolRef&) = delete;                ///< Deleted copy assignment operator.
    BoolRef& operator=(BoolRef&&) = delete;                     ///< Deleted move assignment operator.
    //
    bool operator=(bool pValue);                                ///< Assign a value to the referenced bit.
    //
    operator bool() const;                                      ///< Get the value of the referenced bit.
    //
    bool get() const;                                           ///< Get the value of the referenced bit.

private:
    using BitsetRef = std::reference_wrapper<boost::dynamic_bitset<>>;  ///< Wrapper for reference to bitset to be held by \c std::variant.
    using FieldRef = std::reference_wrapper<RegField>;                  ///< Wrapper for reference to RegField to be held by \c std::variant.

private:
    const std::variant<const BitsetRef, const FieldRef> dataField;  ///< Referenced dataset (either raw bitset or abstract register field).
    const std::size_t idx;                                          ///< Index of the referenced bit in the referenced register field.
};

/*!
 * \brief Proxy class for accessing an individual register field.
 *
 * Provides convenient read/write access to a certain register \e field, a subset of
 * bits from a StandardRegister instance or of another, parent register field thereof.
 */
class StandardRegister::RegField                                                        // cppcheck-suppress noConstructor symbolName=RegField
{
private:
    RegField(RegField& pParent, const std::vector<std::size_t>& pIdxs);                                             ///< Constructor.

public:
    RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, std::uint64_t pSize, std::uint64_t pOffs);   ///< Constructor.
    RegField(RegField& pParent, const std::string& pName, std::uint64_t pSize, std::uint64_t pOffs,
             const std::vector<std::uint64_t>& pBitOrder = {});                                                     ///< Constructor.
    RegField(const RegField& pOther);                                                       ///< Copy constructor.
    RegField(RegField&&) = delete;                                                          ///< Deleted move constructor.
    ~RegField() = default;                                                                  ///< Default destructor.
    //
    RegField& operator=(const RegField&) = delete;                                          ///< Deleted copy assignment operator.
    RegField& operator=(RegField&&) = delete;                                               ///< Deleted move assignment operator.
    //
    std::uint64_t operator=(std::uint64_t pValue);                                          ///< Assign equivalent integer value to the field.
    const boost::dynamic_bitset<>& operator=(const boost::dynamic_bitset<>& pBits);         ///< Assign a raw bit sequence to the field.
    //
    void set(std::uint64_t pValue);                                                         ///< Assign equivalent integer value to the field.
    void set(const boost::dynamic_bitset<>& pBits);                                         ///< Assign a raw bit sequence to the field.
    void setAll(bool pValue = true);                                                        ///< Set/unset all field bits at once.
    //
    explicit operator std::uint64_t() const;                                                ///< Get the integer equivalent of field's content.
    explicit operator boost::dynamic_bitset<>() const;                                      ///< Get the field's content as raw bitset.
    //
    std::uint64_t toUInt() const;                                                           ///< Get the integer equivalent of field's content.
    boost::dynamic_bitset<> toBits() const;                                                 ///< Get the field's data as raw bitset.
    //
    RegField& operator[](std::string_view pFieldName);                                      ///< Access an immediate child field.
    const RegField& operator[](std::string_view pFieldName) const;                          ///< Access an immediate child field.
    BoolRef& operator[](std::size_t pIdx);                                                  ///< Access a specific bit in the field.
    const BoolRef& operator[](std::size_t pIdx) const;                                      ///< Access a specific bit in the field.
    RegField operator()(std::size_t pMsbIdx, std::size_t pLsbIdx);                          ///< Access a slice of bits in the field.
    RegField operator[](const std::vector<std::size_t>& pIdxs);                             ///< Access a set of unique bits in the field.
    RegField operator[](std::initializer_list<std::size_t> pIdxs);                          ///< Access a set of unique bits in the field.
    //
    RegField& n(std::size_t pFieldRepIdx);                                                  ///< Access the n-th repetition of the field.
    const RegField& n(std::size_t pFieldRepIdx) const;                                      ///< Access the n-th repetition of the field.
    //
    std::uint64_t getSize() const;                                                          ///< Get the size of the field.
    std::uint64_t getOffset() const;                                                        ///< \brief Get the field's offset
                                                                                            ///  with respect to its parent field.
    std::uint64_t getTotalOffset() const;                                                   ///< \brief Get the field's total offset
                                                                                            ///  with respect to the whole register.

private:
    void setChildFields(std::map<std::string, const std::reference_wrapper<RegField>, std::less<>> pChildFields);
                                                                                            ///< Set references to the immediate child fields.
    void setChildFields(const std::vector<std::pair<std::string, const std::reference_wrapper<RegField>>>& pFieldReps);
                                                                            ///< Assign field repetition numbers to actual child field names.
    //
    /// \cond INTERNAL
    /*!
     * \brief Let the enclosing StandardRegister class set the field's child
     * fields \e after it has recursively constructed the nested fields branch.
     *
     * See setChildFields() and StandardRegister::populateFieldTree() / StandardRegister::StandardRegister().
     */
    friend StandardRegister::StandardRegister(std::string, HL::Driver&, LayerConfig);
    friend void StandardRegister::populateFieldTree(StandardRegister::FieldTree&, const boost::property_tree::ptree&, const std::string&) const;
    /// \endcond INTERNAL

private:
    const std::string name;                     ///< Name of the field.
    //
    const std::uint64_t size;                   ///< Size of the field in number of bits.
    const std::uint64_t offs;                   ///< Index of the field's most significant bit in the parent field.
    //
    const std::uint64_t parentSize;             ///< Size of the parent field in number of bits.
    const std::uint64_t parentTotalOffs;        ///< Index of the parent field's most significant bit in the register's top level bitset.
    //
    const std::vector<std::unique_ptr<BoolRef>> dataRefs;   ///< Proxy references to all of the field's bits (least significant bit at front).
    //
    std::map<std::string, const std::reference_wrapper<RegField>, std::less<>> childFields;     ///< Map of immediate child fields.
    std::vector<std::string> repetitionKeys;    ///< Assignment of number of field's repetition to respective key in 'childFields'.
};

} // namespace Layers::RL

} // namespace casil

#endif // CASIL_LAYERS_RL_STANDARDREGISTER_H
