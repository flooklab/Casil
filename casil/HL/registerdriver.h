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

#ifndef CASIL_HL_REGISTERDRIVER_H
#define CASIL_HL_REGISTERDRIVER_H

#include <casil/HL/muxeddriver.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace casil
{

namespace HL
{

/*!
 * \brief Specification for a register accessed via RegisterDriver.
 *
 * \todo Detailed doc
 */
struct RegisterDescr
{
    /*!
     * \brief Data type modeled by the register.
     *
     * \todo Detailed doc
     */
    enum class DataType : std::uint8_t
    {
        Value = 0,      ///< Numerical value.
        ByteArray = 1   ///< Sequence of individual bytes.
    };
    /*!
     * \brief Defines read-/write-ability of the register.
     *
     * \todo Detailed doc
     */
    enum class AccessMode : std::uint8_t
    {
        ReadOnly = 0b01,    ///< Can only read from the register.
        WriteOnly = 0b10,   ///< Can only write to the register.
        ReadWrite = 0b11    ///< Can read from and write to the register.
    };
    //
    typedef std::variant<std::monostate, std::uint64_t, std::vector<std::uint8_t>> VariantValueType;
    //
    const DataType type = DataType::Value;
    const AccessMode mode = AccessMode::ReadWrite;
    const std::uint32_t addr = 0;
    const std::uint32_t size = 0;
    const std::uint32_t offs = 0;
    const VariantValueType defaultValue = std::monostate{};
};

/*!
 * \brief Specialization for principally MuxedDriver components that will mainly control their firmware module via register operations.
 *
 * \todo Detailed doc
 */
class RegisterDriver : public MuxedDriver
{
public:
    using MuxedDriver::InterfaceBaseType;

protected:
    using DataType = RegisterDescr::DataType;
    using AccessMode = RegisterDescr::AccessMode;

public:
    class RegisterProxy;

public:
    RegisterDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                   LayerConfig pConfig, const LayerConfig& pRequiredConfig, std::map<std::string, RegisterDescr, std::less<>> pRegisters);
    ~RegisterDriver() override = default;
    //
    const RegisterProxy& operator[](std::string_view pRegName) const;
    //
    void reset() override final;
    //
    void applyDefaults();
    //
    std::vector<std::uint8_t> getBytes(std::string_view pRegName);
    void setBytes(std::string_view pRegName, const std::vector<std::uint8_t>& pData);
    //
    std::uint64_t getValue(std::string_view pRegName);
    void setValue(std::string_view pRegName, std::uint64_t pValue);
    //
    std::variant<std::uint64_t, std::vector<std::uint8_t>> get(std::string_view pRegName);
    void set(std::string_view pRegName, std::uint64_t pValue);
    void set(std::string_view pRegName, const std::vector<std::uint8_t>& pBytes);
    //
    void trigger(std::string_view pRegName);
    //
    bool testRegisterName(std::string_view pRegName) const;
    //
    static bool isValidRegisterName(std::string_view pRegName);

private:
    bool initImpl() override final;
    bool closeImpl() override final;
    //
    virtual bool initModule();
    virtual bool closeModule();
    //
    virtual void resetImpl() = 0;
    //
    virtual std::uint8_t getModuleSoftwareVersion() const = 0;
    virtual std::uint8_t getModuleFirmwareVersion() = 0;
    virtual bool checkVersionRequirement(std::uint8_t pSoftwareVersion, std::uint8_t pFirmwareVersion);
    bool checkVersionRequirement();
    //
    std::vector<std::uint8_t> getRegBytes(std::uint32_t pRegAddr, std::uint32_t pRegSize) const;
    void setRegBytes(std::uint32_t pRegAddr, const std::vector<std::uint8_t>& pData) const;
    std::uint64_t getRegValue(std::uint32_t pRegAddr, std::uint32_t pRegSize, std::uint32_t pRegOffs) const;
    void setRegValue(std::uint32_t pRegAddr, std::uint32_t pRegSize, std::uint32_t pRegOffs, std::uint64_t pValue) const;

protected:
    const bool clearRegValCacheOnReset;
    //
    const std::map<std::string, RegisterDescr, std::less<>> registers;

private:
    std::map<std::string, RegisterDescr::VariantValueType, std::less<>> registerWrittenCache;
    std::map<std::string, RegisterDescr::VariantValueType, std::less<>> initValues;

public:
    /*!
     * \brief Proxy class for accessing an individual RegisterDriver register.
     *
     * \todo Detailed doc
     */
    class RegisterProxy                                                         // cppcheck-suppress noConstructor symbolName=RegisterProxy
    {
    public:
        RegisterProxy(RegisterDriver& pRegDriver, std::string pRegName);
        RegisterProxy(const RegisterProxy&) = delete;
        RegisterProxy(RegisterProxy&&) = delete;
        ~RegisterProxy() = default;
        //
        RegisterProxy& operator=(RegisterProxy) = delete;
        RegisterProxy& operator=(RegisterProxy&&) = delete;
        //
        std::uint64_t operator=(std::uint64_t pValue) const;
        const std::vector<std::uint8_t>& operator=(const std::vector<std::uint8_t>& pBytes) const;
        //
        operator std::uint64_t() const;
        operator std::vector<std::uint8_t>() const;
        //
        std::variant<std::uint64_t, std::vector<std::uint8_t>> get() const;
        //
        void trigger() const;

    private:
        RegisterDriver& regDriver;
        const std::string regName;
    };

private:
    std::map<std::string, const RegisterProxy, std::less<>> registerProxies;
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_REGISTERDRIVER_H
