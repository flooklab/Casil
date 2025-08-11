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
 * \brief ...
 *
 * \todo ...
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
    const RegField& operator[](const std::string& pFieldPath) const;                ///< TODO
    const BoolRef& operator[](std::size_t pIdx) const;                              ///< TODO
    //
    const RegField& root() const;                                                   ///< TODO
    //
    std::uint64_t getSize() const;                                                  ///< Get the size of the register.
/*    //
    void set(const boost::dynamic_bitset<>& pBits);
    boost::dynamic_bitset<> getWr() const;
    boost::dynamic_bitset<> getRd() const;
    //
    void write() const;
    void read() const;*/

private:
    bool initImpl() override;                                                       ///< TODO
    bool closeImpl() override;                                                      ///< TODO
    //
    typedef boost::property_tree::basic_ptree<std::string, std::shared_ptr<RegField>> FieldTree;    ///< TODO
    void populateFieldTree(FieldTree& pFieldTree, const boost::property_tree::ptree& pConfTree, const std::string& pParentKey) const;
                                                                                    ///< TODO

private:
    const std::uint64_t size;       ///< TODO
    //
    boost::dynamic_bitset<> data;   ///< TODO
    //
    FieldTree fields;               ///< TODO

    CASIL_REGISTER_REGISTER_H("StandardRegister")
};

/*!
 * \brief The StandardRegister::BoolRef class
 *
 * \todo ...
 */
class StandardRegister::BoolRef
{
public:
    BoolRef(boost::dynamic_bitset<>& pBits, std::size_t pIdx);  ///< Constructor.
    BoolRef(const RegField& pParent, std::size_t pIdx);         ///< Constructor.
    BoolRef(const BoolRef&) = default;                          ///< Default copy constructor.
    BoolRef(BoolRef&&) = delete;                                ///< Deleted move constructor.
    ~BoolRef() = default;                                       ///< Default destructor.
    //
    BoolRef& operator=(const BoolRef&) = delete;                ///< Deleted copy assignment operator.
    BoolRef& operator=(BoolRef&&) = delete;                     ///< Deleted move assignment operator.
    //
    bool operator=(bool pValue) const;                          ///< TODO
    //
    operator bool() const;                                      ///< TODO
    //
    bool get() const;                                           ///< TODO

private:
    using BitsetRef = std::reference_wrapper<boost::dynamic_bitset<>>;
    using FieldRef = std::reference_wrapper<const RegField>;

private:
    const std::variant<const BitsetRef, const FieldRef> dataField;  ///< TODO
    const std::size_t idx;                                          ///< TODO
};

/*!
 * \brief The StandardRegister::RegField class
 *
 * \todo ...
 */
class StandardRegister::RegField
{
public:
    RegField(boost::dynamic_bitset<>& pBits, const std::string& pName, std::uint64_t pSize, std::uint64_t pOffs);   ///< Constructor.
    RegField(const RegField& pParent, const std::string& pName, std::uint64_t pSize, std::uint64_t pOffs);          ///< Constructor.
    RegField(const RegField&) = delete;                                                     ///< Deleted copy constructor.
    RegField(RegField&&) = delete;                                                          ///< Deleted move constructor.
    ~RegField() = default;                                                                  ///< Default destructor.
    //
    RegField& operator=(const RegField&) = delete;                                          ///< Deleted copy assignment operator.
    RegField& operator=(RegField&&) = delete;                                               ///< Deleted move assignment operator.
    //
    std::uint64_t operator=(std::uint64_t pValue) const;                                    ///< TODO
    const boost::dynamic_bitset<>& operator=(const boost::dynamic_bitset<>& pBits) const;   ///< TODO
    //
    explicit operator std::uint64_t() const;                                                ///< TODO
    explicit operator boost::dynamic_bitset<>() const;                                      ///< TODO
    //
    std::uint64_t toUInt() const;                                                           ///< TODO
    boost::dynamic_bitset<> toBits() const;                                                 ///< TODO
    //
    const RegField& operator[](const std::string_view pFieldName) const;                    ///< TODO
    const BoolRef& operator[](std::size_t pIdx) const;                                      ///< TODO
    //
    const RegField& n(std::size_t pFieldRepIdx) const;                                      ///< TODO
    //
    std::uint64_t getSize() const;                                                          ///< TODO
    std::uint64_t getOffset() const;                                                        ///< TODO

private:
    void setChildFields(std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> pChildFields);
                                                                                            ///< TODO
    void setChildFields(const std::vector<std::pair<std::string, const std::reference_wrapper<const RegField>>>& pFieldReps);
                                                                                            ///< TODO
    //
    //Let enclosing StandardRegister class set children after recursively constructing nested fields branch
    friend void StandardRegister::populateFieldTree(StandardRegister::FieldTree&, const boost::property_tree::ptree&, const std::string&) const;

private:
    const std::string name;                     ///< TODO
    //
    const std::uint64_t size;                   ///< TODO
    const std::uint64_t offs;                   ///< TODO
    //
    const std::vector<BoolRef> dataRefs;        ///< TODO
    //
    std::map<std::string, const std::reference_wrapper<const RegField>, std::less<>> childFields;   ///< TODO
    std::vector<std::string> repetitionKeys;    ///< TODO
};

} // namespace Layers::RL

} // namespace casil

#endif // CASIL_LAYERS_RL_STANDARDREGISTER_H
