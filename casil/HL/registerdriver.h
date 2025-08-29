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

#ifndef CASIL_LAYERS_HL_REGISTERDRIVER_H
#define CASIL_LAYERS_HL_REGISTERDRIVER_H

#include <casil/HL/muxeddriver.h>

#include <casil/layerconfig.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace casil
{

namespace Layers
{

namespace HL
{

/*!
 * \brief Specification for a register accessed via RegisterDriver.
 *
 * Defines the essential properties of a typical firmware module register (such as size, alignment etc.) except its name.
 */
struct RegisterDescr
{
    /*!
     * \brief Data type modeled by the register.
     *
     * A single register can either represent a single number/value/variable or an array/sequence of a number of individual bytes.
     */
    enum class DataType : std::uint8_t
    {
        Value = 0,      ///< Numerical value.
        ByteArray = 1   ///< Sequence of individual bytes.
    };
    /*!
     * \brief Defines read-/write-ability of the register.
     */
    enum class AccessMode : std::uint8_t
    {
        ReadOnly = 0b01,    ///< Can only read from the register.
        WriteOnly = 0b10,   ///< Can only write to the register.
        ReadWrite = 0b11    ///< Can read from and write to the register.
    };
    //
    typedef std::variant<std::monostate, std::uint64_t, std::vector<std::uint8_t>> VariantValueType;
                                                            ///< Variant to optionally store the two possible types of register content.
    //
    const DataType type = DataType::Value;                  ///< \copybrief DataType
    const AccessMode mode = AccessMode::ReadWrite;          ///< \copybrief AccessMode
    const std::uint32_t addr = 0;                           ///< %Register address in bytes.
    const std::uint32_t size = 0;                           ///< %Register size (in bits for DataType::Value, in bytes for DataType::ByteArray).
    const std::uint32_t offs = 0;                           ///< %Register bit offset from its \ref addr "address" (only for DataType::Value).
    const VariantValueType defaultValue = std::monostate{}; ///< Designated default register content.
};

/*!
 * \brief Specialization for principally MuxedDriver components that will mainly control their firmware module via register operations.
 *
 * Use this class as base class for implementing a "register-dominated" "muxed" driver, i.e. a driver
 * which controls a "register-dominated" firmware module running on FPGA hardware based on the basil bus,
 * which in turn is connected to via a "muxed" interface. Here, "register-dominated" means that controlling
 * the firmware module from the driver happens (mostly) via standardized reading/writing to/from \e registers.
 *
 * Consider to instead use the simpler MuxedDriver as base class if such register operations are not needed.
 *
 * Note: This class and MuxedDriver are both in contrast to DirectDriver, which should be
 * used for independent/stand-alone hardware that is connected to via a "direct" interface.
 */
class RegisterDriver : public MuxedDriver
{
public:
    using MuxedDriver::InterfaceBaseType;

protected:
    //Declaring these aliases to make definition of registers in implementations more compact
    using DataType = RegisterDescr::DataType;       ///< \copybrief RegisterDescr::DataType
    using AccessMode = RegisterDescr::AccessMode;   ///< \copybrief RegisterDescr::AccessMode

public:
    class RegisterProxy;

public:
    RegisterDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                   LayerConfig pConfig, const LayerConfig& pRequiredConfig, std::map<std::string, RegisterDescr, std::less<>> pRegisters);
                                            ///< Constructor.
    ~RegisterDriver() override = default;   ///< Default destructor.
    //
    const RegisterProxy& operator[](std::string_view pRegName) const;   ///< Access a register via the proxy class.
    //
    void reset() override final;                                        ///< Reset the firmware module.
    //
    void applyDefaults();                                               ///< Write configured default values to all appropriate registers.
    //
    std::vector<std::uint8_t> getBytes(std::string_view pRegName);                          ///< Read the data from a byte array register.
    void setBytes(std::string_view pRegName, const std::vector<std::uint8_t>& pData);       ///< Write data to a byte array register.
    //
    std::uint64_t getValue(std::string_view pRegName);                                      ///< Read the value from a value register.
    void setValue(std::string_view pRegName, std::uint64_t pValue);                         ///< Write a value to a value register.
    //
    std::variant<std::uint64_t, std::vector<std::uint8_t>> get(std::string_view pRegName);  ///< \brief Read an integer or byte sequence from
                                                                                            ///  a register, according to its data type.
    void set(std::string_view pRegName, std::uint64_t pValue);                              ///< Write a value to a value register.
    void set(std::string_view pRegName, const std::vector<std::uint8_t>& pBytes);           ///< Write data to a byte array register.
    //
    void trigger(std::string_view pRegName);                        ///< "Trigger" a write-only register by writing configured default or zero.
    //
    bool testRegisterName(std::string_view pRegName) const;         ///< Check if a register exists or throw an exception else.
    //
    static bool isValidRegisterName(std::string_view pRegName);     ///< Check if a string could be a valid register name.

private:
    bool initImpl() override final;
    bool closeImpl() override final;
    //
    void loadRuntimeConfImpl(boost::property_tree::ptree&& pConf) override final;
    boost::property_tree::ptree dumpRuntimeConfImpl() const override final;
    //
    virtual bool initModule();          ///< Perform module-specific initialization steps.
    virtual bool closeModule();         ///< Perform module-specific closing steps.
    //
    virtual void resetImpl() = 0;       ///< Perform the module-specific reset sequence for reset().
    //
    /*!
     * \brief Get the driver software version.
     *
     * \return Software version number.
     */
    virtual std::uint8_t getModuleSoftwareVersion() const = 0;
    /*!
     * \brief Read the module firmware version.
     *
     * \return Firmware version number.
     */
    virtual std::uint8_t getModuleFirmwareVersion() = 0;
    virtual bool checkVersionRequirement(std::uint8_t pSoftwareVersion, std::uint8_t pFirmwareVersion);
                                                                            ///< Check if software version is compatible with firmware version.
    bool checkVersionRequirement();                                         ///< \copybrief checkVersionRequirement(std::uint8_t, std::uint8_t)
    //
    std::vector<std::uint8_t> getRegBytes(std::uint32_t pRegAddr, std::uint32_t pRegSize) const;
                                                                                            ///< Read a byte sequence from a register address.
    void setRegBytes(std::uint32_t pRegAddr, const std::vector<std::uint8_t>& pData) const; ///< Write a byte sequence to a register address.
    std::uint64_t getRegValue(std::uint32_t pRegAddr, std::uint32_t pRegSize, std::uint32_t pRegOffs) const;
                                                                                            ///< Read an integer value from a register address.
    void setRegValue(std::uint32_t pRegAddr, std::uint32_t pRegSize, std::uint32_t pRegOffs, std::uint64_t pValue) const;
                                                                                            ///< Write an integer value to a register address.

protected:
    const bool clearRegValCacheOnReset;                                     ///< Whether to clear the register written value cache on reset().
    //
    const std::map<std::string, RegisterDescr, std::less<>> registers;      ///< Map of all registers with their names as keys.

private:
    std::map<std::string, RegisterDescr::VariantValueType, std::less<>> registerWrittenCache;   ///< Cache of last written register values.
    std::map<std::string, RegisterDescr::VariantValueType> initValues;      ///< Overridden default values from YAML configuration.

public:
    /*!
     * \brief Proxy class for accessing an individual RegisterDriver register.
     *
     * Provides convenient read/write access to a single register of a specific RegisterDriver instance.
     */
    class RegisterProxy                                                         // cppcheck-suppress noConstructor symbolName=RegisterProxy
    {
    public:
        RegisterProxy(RegisterDriver& pRegDriver, std::string pRegName);        ///< Constructor.
        RegisterProxy(const RegisterProxy&) = delete;                           ///< Deleted copy constructor.
        RegisterProxy(RegisterProxy&&) = delete;                                ///< Deleted move constructor.
        ~RegisterProxy() = default;                                             ///< Default destructor.
        //
        RegisterProxy& operator=(RegisterProxy) = delete;                       ///< Deleted copy assignment operator.
        RegisterProxy& operator=(RegisterProxy&&) = delete;                     ///< Deleted move assignment operator.
        //
        std::uint64_t operator=(std::uint64_t pValue) const;                    ///< Write an integer value to the register.
        const std::vector<std::uint8_t>& operator=(const std::vector<std::uint8_t>& pBytes) const;  ///< Write a byte sequence to the register.
        //
        operator std::uint64_t() const;                                         ///< Read an integer value from the register.
        operator std::vector<std::uint8_t>() const;                             ///< Read a byte sequence from the register.
        //
        std::variant<std::uint64_t, std::vector<std::uint8_t>> get() const;     ///< \brief Read an integer or byte sequence from
                                                                                ///  the register, according to its data type.
        //
        void trigger() const;                                   ///< "Trigger" the (write-only) register by writing configured default or zero.

    private:
        RegisterDriver& regDriver;      ///< %Driver to which the register belongs.
        const std::string regName;      ///< Name of the register as used in the driver.
    };

private:
    std::map<std::string, const RegisterProxy, std::less<>> registerProxies;    ///< Map of proxy class instances with register names as keys.
};

} // namespace HL

} // namespace Layers

} // namespace casil

#endif // CASIL_LAYERS_HL_REGISTERDRIVER_H
