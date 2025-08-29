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

#include <casil/HL/registerdriver.h>

#include <casil/auxil.h>
#include <casil/bytes.h>

#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <bitset>
#include <cctype>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>

using casil::Layers::HL::RegisterDriver;

/*!
 * \brief Constructor.
 *
 * Constructs MuxedDriver by forwarding \p pType, \p pName, \p pInterface, \p pConfig and \p pRequiredConfig.
 *
 * Gets the optional "clear_cache_after_reset" value from \p pConfig (boolean value, default: false),
 * which defines, whether to clear the cache for written register values on reset (see reset()).
 *
 * Configures the registers for this driver from the passed definitions in \p pRegisters and checks
 * for potentially invalid register configurations (see the documented exceptions and also RegisterDescr).
 * Also gets any optional default values "init.REG_NAME" for these registers (which override any defaults
 * from \p pRegisters) from \p pConfig (unsigned integers or byte sequences, depending on RegisterDescr::DataType).
 * Init values from \p pConfig that do \e not match any of the registers will be ignored.
 *
 * \throws std::runtime_error If an invalid name is set for one of the registers.
 * \throws std::runtime_error If the size is set to zero for one of the registers.
 * \throws std::runtime_error If the size is larger than 64 bit for one of the value registers.
 * \throws std::runtime_error If the offset is non-zero for one of the byte array registers.
 * \throws std::runtime_error If a default value is set for one of the read-only registers.
 * \throws std::runtime_error If the type of the default value does not match the register's data type for one of the registers.
 * \throws std::runtime_error If the length of the default byte sequence does not match the register size for one of the byte array registers.
 * \throws std::runtime_error If an init value is set for one of the read-only registers.
 * \throws std::runtime_error If the type of the init value (default override) from \p pConfig
 *                            does not match the register's data type for one of the registers.
 * \throws std::runtime_error If the length of the init byte sequence (default override) from \p pConfig
 *                            does not match the register size for one of the byte array registers.
 * \throws std::runtime_error If the init value (default override) from \p pConfig cannot be parsed at all
 *                            (does not match any of the supported data types) for one of the registers.
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 * \param pRegisters Map of needed registers (their names as keys) to be defined by the derived class (the actual driver component).
 */
RegisterDriver::RegisterDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                               LayerConfig pConfig, const LayerConfig& pRequiredConfig,
                               std::map<std::string, RegisterDescr, std::less<>> pRegisters) :
    MuxedDriver(std::move(pType), std::move(pName), pInterface, std::move(pConfig), pRequiredConfig),
    clearRegValCacheOnReset(config.getBool("clear_cache_after_reset", false)),
    registers(std::move(pRegisters)),
    registerWrittenCache(),
    initValues(),
    registerProxies()
{
    for (const auto& [regName, regDescr] : registers)
    {
        //Check for invalid register settings

        if (!isValidRegisterName(regName))
        {
            throw std::runtime_error("Invalid name set for register \"" + regName + "\" of register driver \"" + name + "\": " +
                                     "Must contain only uppercase letters and underscores.");
        }
        if (regDescr.size == 0)
        {
            throw std::runtime_error("Size is set to zero for register \"" + regName + "\" of register driver \"" + name + "\".");
        }
        if (regDescr.type == DataType::Value && regDescr.size > 64)
        {
            throw std::runtime_error("Size is larger than 64 bit for value register \"" + regName + "\" " +
                                     "of register driver \"" + name + "\".");
        }
        if (regDescr.type == DataType::ByteArray && regDescr.offs > 0)
        {
            throw std::runtime_error("Offset is non-zero for byte array register \"" + regName + "\" "+
                                     "of register driver \"" + name + "\".");
        }
        if (regDescr.mode == AccessMode::ReadOnly && !std::holds_alternative<std::monostate>(regDescr.defaultValue))
        {
            throw std::runtime_error("Default value set for read-only register \"" + regName + "\" of register driver "+
                                     "\"" + name + "\".");
        }
        if ((regDescr.type == DataType::Value && std::holds_alternative<std::vector<std::uint8_t>>(regDescr.defaultValue)) ||
            (regDescr.type == DataType::ByteArray && std::holds_alternative<std::uint64_t>(regDescr.defaultValue)))
        {
            throw std::runtime_error("Default value type does not match data type for register \"" + regName + "\" " +
                                     "of register driver \"" + name + "\".");
        }
        if (std::holds_alternative<std::vector<std::uint8_t>>(regDescr.defaultValue) &&
            std::get<std::vector<std::uint8_t>>(regDescr.defaultValue).size() != regDescr.size)
        {
            throw std::runtime_error("Default byte sequence length does not match register size for register \"" + regName + "\" "
                                     "of register driver \"" + name + "\".");
        }

        //Initialize register written value cache
        if (regDescr.mode != AccessMode::ReadOnly)
            registerWrittenCache[regName] = std::monostate{};

        //Collect manually overridden default values from "init" map of configuration YAML

        initValues[regName] = std::monostate{};

        //Cannot use init entries for read-only registers
        if (regDescr.mode == AccessMode::ReadOnly && config.contains(LayerConfig::fromYAML("{init: {" + regName + ": }}"), false))
        {
            throw std::runtime_error("Init value set for read-only register \"" + regName + "\" of register driver \"" + name + "\".");
        }

        if (config.contains(LayerConfig::fromYAML("{init: {" + regName + ": uint}}"), true))
        {
            if (regDescr.type == DataType::ByteArray)
            {
                throw std::runtime_error("Init value type does not match data type for register \"" + regName + "\" " +
                                         "of register driver \"" + name + "\".");
            }

            initValues[regName] = config.getUInt("init." + regName);
        }
        else if (config.contains(LayerConfig::fromYAML("{init: {" + regName + ": byteSeq}}"), true))
        {
            if (regDescr.type == DataType::Value)
            {
                throw std::runtime_error("Init value type does not match data type for register \"" + regName + "\" " +
                                         "of register driver \"" + name + "\".");
            }

            std::vector<std::uint8_t> seqVec = config.getByteSeq("init." + regName);

            if (seqVec.size() != regDescr.size)
            {
                throw std::runtime_error("Init byte sequence for register \"" + regName + "\" of register driver \"" + name + "\" " +
                                         "has wrong size.");
            }

            initValues[regName] = std::move(seqVec);
        }
        else if (config.contains(LayerConfig::fromYAML("{init: {" + regName + ": }}"), false))
        {
            //Contains an entry for the register name but with invalid value / wrong type
            throw std::runtime_error("Could not parse init value for register \"" + regName + "\" of register driver \"" + name + "\".");
        }

        //Initialize register proxies
        registerProxies.emplace(std::piecewise_construct, std::forward_as_tuple(regName), std::forward_as_tuple(*this, regName));
    }
}

//Public

/*!
 * \brief Access a register via the proxy class.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 *
 * \param pRegName Name of the register.
 * \return Proxy class instance for register \p pRegName.
 */
const RegisterDriver::RegisterProxy& RegisterDriver::operator[](const std::string_view pRegName) const
{
    const auto it = registerProxies.find(pRegName);

    if (it == registerProxies.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    return it->second;
}

//

/*!
 * \brief Reset the firmware module.
 *
 * Performs a driver-specific reset sequence for the module.
 *
 * \internal This logic is to be implemented by resetImpl(). \endinternal
 *
 * Then, if "clear_cache_after_reset" was enabled in the component configuration, the
 * cache for previously written register values will be cleared (see RegisterDriver()).
 */
void RegisterDriver::reset()
{
    resetImpl();

    if (clearRegValCacheOnReset)
    {
        for (auto& it : registerWrittenCache)
            it.second = std::monostate{};
    }
}

//

/*!
 * \brief Write configured default values to all appropriate registers.
 *
 * The default value/sequence will be written to every writable register that has a default, which can be either
 * from its definition (see RegisterDriver(), RegisterDescr) or from the possible "init.REG_NAME" overrides in
 * the component configuration (see RegisterDriver()). If the latter is present, it will be used, by preference.
 */
void RegisterDriver::applyDefaults()
{
    for (const auto& [regName, regDescr] : registers)
    {
        if (regDescr.mode == AccessMode::ReadOnly)
            continue;

        if (!std::holds_alternative<std::monostate>(initValues[regName]))
        {
            if (std::holds_alternative<std::uint64_t>(initValues[regName]))
                setValue(regName, std::get<std::uint64_t>(initValues[regName]));
            else if (std::holds_alternative<std::vector<std::uint8_t>>(initValues[regName]))
                setBytes(regName, std::get<std::vector<std::uint8_t>>(initValues[regName]));
        }
        else if (!std::holds_alternative<std::monostate>(regDescr.defaultValue))
        {
            if (std::holds_alternative<std::uint64_t>(regDescr.defaultValue))
                setValue(regName, std::get<std::uint64_t>(regDescr.defaultValue));
            else if (std::holds_alternative<std::vector<std::uint8_t>>(regDescr.defaultValue))
                setBytes(regName, std::get<std::vector<std::uint8_t>>(regDescr.defaultValue));
        }
    }
}

//

/*!
 * \brief Read the data from a byte array register.
 *
 * Reads the byte sequence for register \p pRegName from the firmware module.
 *
 * Silently redirects to trigger() (and returns an empty sequence) if \p pRegName is a write-only register.
 *
 * Note: Warns via Logger if the read bytes do not match the corresponding sequence from the written values cache.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e value register.
 * \throws std::runtime_error If the read (or (trigger-)write if write-only) fails.
 *
 * \param pRegName Name of the register.
 * \return Byte sequence stored in the register (or empty vector if write-only).
 */
std::vector<std::uint8_t> RegisterDriver::getBytes(const std::string_view pRegName)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.type == DataType::Value)
    {
        throw std::invalid_argument("Cannot read byte sequence from value register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }

    if (reg.mode == AccessMode::WriteOnly)
    {
        trigger(pRegName);
        return {};
    }

    try
    {
        std::vector<std::uint8_t> retVal = getRegBytes(reg.addr, reg.size);

        if (reg.mode == AccessMode::ReadWrite)
        {
            const auto& cachedVal = registerWrittenCache.find(pRegName)->second;

            if (!std::holds_alternative<std::monostate>(cachedVal))
            {
                if (retVal != std::get<std::vector<std::uint8_t>>(cachedVal))
                    logger.logWarning("Byte sequence read from register \"" + std::string(pRegName) + "\" differs from cached one.");
            }
        }

        return retVal;
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not read byte sequence from register \"" + std::string(pRegName) + "\" " +
                                 "of register driver \"" + name + "\": " + exc.what());
    }
}

/*!
 * \brief Read the data from a byte array register.
 *
 * This is the \c const overload for getBytes(std::string_view).
 *
 * Reads the byte sequence for register \p pRegName from the firmware module.
 *
 * Instead of the non-<tt>const</tt> getBytes(std::string_view), this \c const overload (naturally)
 * does \b not redirect to trigger() and does \b not support write-only registers at all.
 *
 * Note: Warns via Logger if the read bytes do not match the corresponding sequence from the written values cache.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e value register.
 * \throws std::invalid_argument If \p pRegName is write-only.
 * \throws std::runtime_error If the read fails.
 *
 * \param pRegName Name of the register.
 * \return Byte sequence stored in the register.
 */
std::vector<std::uint8_t> RegisterDriver::getBytes(const std::string_view pRegName) const
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    if (it->second.mode == AccessMode::WriteOnly)
    {
        throw std::invalid_argument("Cannot read (via const overload) from write-only register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }
    else
        return const_cast<RegisterDriver*>(this)->getBytes(pRegName);   //Cast safe because write-only trigger case excluded
}

/*!
 * \brief Write data to a byte array register.
 *
 * Writes \p pData to the register \p pRegName in the firmware module.
 *
 * Saves \p pData in the written value cache on success.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e value register.
 * \throws std::invalid_argument If \p pRegName is read-only.
 * \throws std::invalid_argument If size of \p pData does not match size of \p pRegName.
 * \throws std::runtime_error If the write fails.
 *
 * \param pRegName Name of the register.
 * \param pData Byte sequence to be written.
 */
void RegisterDriver::setBytes(const std::string_view pRegName, const std::vector<std::uint8_t>& pData)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.type == DataType::Value)
    {
        throw std::invalid_argument("Cannot write byte sequence to value register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }
    if (reg.mode == AccessMode::ReadOnly)
    {
        throw std::invalid_argument("Cannot write to read-only register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }
    if (pData.size() != reg.size)
    {
        throw std::invalid_argument("Cannot write wrong number of bytes to register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }

    setRegBytes(reg.addr, pData);

    registerWrittenCache.find(pRegName)->second = pData;
}

//

/*!
 * \brief Read the value from a value register.
 *
 * Reads the unsigned integer value stored by register \p pRegName from the firmware module.
 *
 * Silently redirects to trigger() (and returns zero) if \p pRegName is a write-only register.
 *
 * Note: Warns via Logger if the read value does not match the corresponding value from the written values cache.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e byte \e array register.
 * \throws std::runtime_error If the read (or (trigger-)write if write-only) fails.
 *
 * \param pRegName Name of the register.
 * \return Value stored in the register (or zero if write-only).
 */
std::uint64_t RegisterDriver::getValue(const std::string_view pRegName)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.type == DataType::ByteArray)
    {
        throw std::invalid_argument("Cannot read value from byte array register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }

    if (reg.mode == AccessMode::WriteOnly)
    {
        trigger(pRegName);
        return 0;
    }

    try
    {
        std::uint64_t retVal = getRegValue(reg.addr, reg.size, reg.offs);

        if (reg.mode == AccessMode::ReadWrite)
        {
            const auto& cachedVal = registerWrittenCache.find(pRegName)->second;

            if (!std::holds_alternative<std::monostate>(cachedVal))
            {
                if (retVal != std::get<std::uint64_t>(cachedVal))
                    logger.logWarning("Value read from register \"" + std::string(pRegName) + "\" differs from cached value.");
            }
        }

        return retVal;
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not read value from register \"" + std::string(pRegName) + "\" " +
                                 "of register driver \"" + name + "\": " + exc.what());
    }
}

/*!
 * \brief Read the value from a value register.
 *
 * This is the \c const overload for getValue(std::string_view).
 *
 * Reads the unsigned integer value stored by register \p pRegName from the firmware module.
 *
 * Instead of the non-<tt>const</tt> getValue(std::string_view), this \c const overload (naturally)
 * does \b not redirect to trigger() and does \b not support write-only registers at all.
 *
 * Note: Warns via Logger if the read value does not match the corresponding value from the written values cache.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e byte \e array register.
 * \throws std::invalid_argument If \p pRegName is write-only.
 * \throws std::runtime_error If the read fails.
 *
 * \param pRegName Name of the register.
 * \return Value stored in the register.
 */
std::uint64_t RegisterDriver::getValue(const std::string_view pRegName) const
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    if (it->second.mode == AccessMode::WriteOnly)
    {
        throw std::invalid_argument("Cannot read (via const overload) from write-only register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }
    else
        return const_cast<RegisterDriver*>(this)->getValue(pRegName);   //Cast safe because write-only trigger case excluded
}

/*!
 * \brief Write a value to a value register.
 *
 * Writes \p pValue to the register \p pRegName in the firmware module.
 *
 * Saves \p pValue in the written value cache on success.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \p pRegName is a \e byte \e array register.
 * \throws std::invalid_argument If \p pRegName is read-only.
 * \throws std::runtime_error If the write fails.
 *
 * \param pRegName Name of the register.
 * \param pValue Value to be written.
 */
void RegisterDriver::setValue(const std::string_view pRegName, const std::uint64_t pValue)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.type == DataType::ByteArray)
    {
        throw std::invalid_argument("Cannot write value to byte array register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }
    if (reg.mode == AccessMode::ReadOnly)
    {
        throw std::invalid_argument("Cannot write to read-only register \"" + std::string(pRegName) + "\" " +
                                    "of register driver \"" + name + "\".");
    }

    try
    {
        setRegValue(reg.addr, reg.size, reg.offs, pValue);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write value to register \"" + std::string(pRegName) + "\" " +
                                 "of register driver \"" + name + "\": " + exc.what());
    }

    registerWrittenCache.find(pRegName)->second = pValue;
}

//

/*!
 * \brief Read an integer or byte sequence from a register, according to its data type.
 *
 * See getValue(std::string_view) and getBytes(std::string_view).
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::runtime_error If getValue(std::string_view) or getBytes(std::string_view) throw \c std::runtime_error.
 *
 * \param pRegName Name of the register.
 * \return Integer value or byte sequence, depending on the \ref RegisterDescr::DataType "DataType" of \p pRegName.
 */
std::variant<std::uint64_t, std::vector<std::uint8_t>> RegisterDriver::get(const std::string_view pRegName)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.type == RegisterDescr::DataType::Value)
        return getValue(pRegName);
    else
        return getBytes(pRegName);
}

/*!
 * \brief Write a value to a value register.
 *
 * See setValue().
 *
 * \throws std::invalid_argument If setValue() throws \c std::invalid_argument.
 * \throws std::runtime_error If setValue() throws \c std::runtime_error.
 *
 * \param pRegName Name of the register.
 * \param pValue Value to be written.
 */
void RegisterDriver::set(const std::string_view pRegName, const std::uint64_t pValue)
{
    setValue(pRegName, pValue);
}

/*!
 * \brief Write data to a byte array register.
 *
 * See setBytes().
 *
 * \throws std::invalid_argument If setBytes() throws \c std::invalid_argument.
 *
 * \param pRegName Name of the register.
 * \param pBytes Byte sequence to be written.
 */
void RegisterDriver::set(const std::string_view pRegName, const std::vector<std::uint8_t>& pBytes)
{
    setBytes(pRegName, pBytes);
}

//

/*!
 * \brief "Trigger" a write-only register by writing configured default or zero.
 *
 * Calls setValue() or setBytes(), depending on the data type of \p pRegName, in order to write the register's default
 * value/sequence, which can be either from its definition (see RegisterDriver(), RegisterDescr) or from the possible
 * "init.REG_NAME" override in the component configuration (see RegisterDriver()). If the latter is present, it will be
 * used, by preference. If there is none of those two, the value zero or a byte sequence consisting of zeros will be written.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 * \throws std::invalid_argument If \ref RegisterDescr::AccessMode "AccessMode" of \p pRegName
 *         is \e not \ref RegisterDescr::AccessMode::WriteOnly "WriteOnly".
 * \throws std::runtime_error If setValue() or setBytes() throw \c std::runtime_error.
 *
 * \param pRegName Name of the register.
 */
void RegisterDriver::trigger(const std::string_view pRegName)
{
    const auto it = registers.find(pRegName);

    if (it == registers.end())
    {
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available " +
                                    "for register driver \"" + name + "\".");
    }

    const RegisterDescr& reg = it->second;

    if (reg.mode != AccessMode::WriteOnly)
    {
        throw std::invalid_argument("Cannot trigger register \"" + std::string(pRegName) + "\" of register driver \"" + name + "\": "+
                                    "Only available for write-only registers.");
    }

    const std::string& regName = it->first;

    if (!std::holds_alternative<std::monostate>(initValues[regName]))
    {
        if (std::holds_alternative<std::uint64_t>(initValues[regName]))
            setValue(regName, std::get<std::uint64_t>(initValues[regName]));
        else if (std::holds_alternative<std::vector<std::uint8_t>>(initValues[regName]))
            setBytes(regName, std::get<std::vector<std::uint8_t>>(initValues[regName]));
    }
    else if (!std::holds_alternative<std::monostate>(reg.defaultValue))
    {
        if (std::holds_alternative<std::uint64_t>(reg.defaultValue))
            setValue(regName, std::get<std::uint64_t>(reg.defaultValue));
        else if (std::holds_alternative<std::vector<std::uint8_t>>(reg.defaultValue))
            setBytes(regName, std::get<std::vector<std::uint8_t>>(reg.defaultValue));
    }
    else
    {
        if (reg.type == DataType::Value)
            setValue(regName, 0x0u);
        else
            setBytes(regName, std::vector<std::uint8_t>(reg.size, 0));
    }
}

//

/*!
 * \brief Check if a register exists or throw an exception else.
 *
 * \throws std::invalid_argument If no register with name \p pRegName is defined.
 *
 * \param pRegName Name of the register.
 * \return True.
 */
bool RegisterDriver::testRegisterName(const std::string_view pRegName) const
{
    if (registers.contains(pRegName))
        return true;
    else
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available for register driver \"" + name + "\".");
}

//

/*!
 * \brief Check if a string could be a valid register name.
 *
 * Allowed characters are uppercase letters and underscores.
 *
 * \param pRegName Potential name of a register.
 * \return True if \p pRegName has a valid register name format.
 */
bool RegisterDriver::isValidRegisterName(const std::string_view pRegName)
{
    auto isValidRegNameChar = [](const std::string::value_type pChar) -> bool
    {
        return (pChar == '_') || (std::isupper(static_cast<unsigned char>(pChar)) != 0);
    };

    return std::find_if_not(pRegName.begin(), pRegName.end(), isValidRegNameChar) == pRegName.end();
}

//Private

/*!
 * \copybrief MuxedDriver::initImpl()
 *
 * Resets the firmware module (see reset()), checks the driver's compatibility with the module (see checkVersionRequirement()),
 * write register defaults (see applyDefaults()) and performs further module-specific initialization steps (see initModule()).
 *
 * \return True if successful.
 */
bool RegisterDriver::initImpl()
{
    try
    {
        reset();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not reset: ") + exc.what());
        return false;
    }

    const std::uint8_t softwareVersion = getModuleSoftwareVersion();
    std::uint8_t firmwareVersion;

    try
    {
        firmwareVersion = getModuleFirmwareVersion();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not determine firmware version of FPGA module: ") + exc.what());
        return false;
    }

    if (!checkVersionRequirement(softwareVersion, firmwareVersion))
    {
        logger.logError("FPGA module reports incompatible firmware version: " + std::to_string(firmwareVersion) +
                        "; driver software version is: " + std::to_string(softwareVersion) + ".");
        return false;
    }

    try
    {
        applyDefaults();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not write default register states: ") + exc.what());
        return false;
    }

    if (!initModule())
        return false;

    logger.logSuccess("Initialized: Using FPGA module at base address " + Bytes::formatHex(baseAddr) + " with firmware version " +
                      std::to_string(firmwareVersion) + " (driver version: " + std::to_string(softwareVersion) + ").");

    return true;
}

/*!
 * \copybrief MuxedDriver::closeImpl()
 *
 * Calls closeModule() and returns accordingly.
 *
 * \return True if successful.
 */
bool RegisterDriver::closeImpl()
{
    if (!closeModule())
        return false;

    return true;
}

//

/*!
 * \copybrief Register::loadRuntimeConfImpl()
 *
 * ...
 *
 * \todo ...
 *
 *
 * \throws std::runtime_error If a node in \p pConf has neither non-empty data nor a child node.
 * \throws std::runtime_error If a node in \p pConf has \e both non-empty data and a child node.
 * \throws std::runtime_error If a register does not exist.
 * \throws std::runtime_error If a register is read-only or write-only.
 * \throws std::runtime_error If the to be set value/sequence does not match the data type of the register.
 * \throws std::runtime_error If the parsing of a value/sequence fails due to invalid formatting.
 * \throws std::runtime_error If a byte sequence length does not match the register size/length.
 * \throws std::runtime_error If writing to the interface fails (see also setValue(), setBytes()).
 *
 * \param pConf Desired runtime configuration tree.
 */
void RegisterDriver::loadRuntimeConfImpl(boost::property_tree::ptree&& pConf)
{
    const LayerConfig tConf(pConf);

    for (const auto& [key, subTree] : pConf)
    {
        if (subTree.empty() && subTree.data() == "")
            throw std::runtime_error("Node has neither non-empty data nor a child node.");
        else if (!subTree.empty() && subTree.data() != "")
            throw std::runtime_error("Node must have either non-empty data or a child node.");

        const auto regIt = registers.find(key);

        if (regIt == registers.end())
            throw std::runtime_error("Register \"" + key + "\" is not available.");

        if (regIt->second.mode != AccessMode::ReadWrite)
        {
            throw std::runtime_error("Register \"" + key + "\" is " +
                                     ((regIt->second.mode == AccessMode::ReadOnly) ? "read" : "write") + "-only.");
        }

        if (subTree.empty())
        {
            if (regIt->second.type != DataType::Value)
                throw std::runtime_error("Integer value is defined for register \"" + key + "\" but byte sequence is required.");

            const std::optional<std::uint64_t> val = tConf.getUIntOpt(key);
            if (val)
                setValue(key, val.value());
            else
                throw std::runtime_error("Could not parse value for register \"" + key + "\".");
        }
        else
        {
            if (regIt->second.type != DataType::ByteArray)
                throw std::runtime_error("Byte sequence is defined for register \"" + key + "\" but integer value is required.");

            const std::optional<std::vector<std::uint8_t>> seq = tConf.getByteSeqOpt(key);
            if (seq)
            {
                try
                {
                    setBytes(key, seq.value());
                }
                catch (const std::invalid_argument& exc)
                {
                    throw std::runtime_error(exc.what());
                }
            }
            else
                throw std::runtime_error("Could not parse byte sequence for register \"" + key + "\".");
        }
    }
}

/*!
 * \copybrief Register::dumpRuntimeConfImpl()
 *
 * ...
 *
 * \todo ...
 *
 * \return Current runtime configuration tree.
 */
boost::property_tree::ptree RegisterDriver::dumpRuntimeConfImpl() const
{
    using boost::property_tree::ptree;
    ptree confTree;

    for (const auto& [regName, regDescr] : registers)
    {
        if (regDescr.mode != AccessMode::ReadWrite)
            continue;

        ptree valTree;

        if (regDescr.type == DataType::Value)
        {
            valTree.data() = Bytes::formatHex(getValue(regName));
        }
        else
        {
            std::size_t byteNum = 0;
            for (const std::uint8_t byte : getBytes(regName))
            {
                ptree numberNode;
                numberNode.data() = Bytes::formatHex(byte);
                valTree.add_child("#" + std::to_string(byteNum++), numberNode);
            }
        }

        confTree.add_child(regName, valTree);
    }

    return confTree;
}

//

/*!
 * \brief Perform module-specific initialization steps.
 *
 * Does nothing (override for specific drivers if needed).
 *
 * \return True.
 */
bool RegisterDriver::initModule()
{
    return true;
}

/*!
 * \brief Perform module-specific closing steps.
 *
 * Does nothing (override for specific drivers if needed).
 *
 * \return True.
 */
bool RegisterDriver::closeModule()
{
    return true;
}

//

/*!
 * \brief Check if software version is compatible with firmware version.
 *
 * Checks for \e equality of \p pSoftwareVersion and \p pFirmwareVersion.
 *
 * \param pSoftwareVersion %Driver software version (as from getModuleSoftwareVersion()).
 * \param pFirmwareVersion Firmware module version (as from getModuleFirmwareVersion()).
 * \return If versions are compatible.
 */
bool RegisterDriver::checkVersionRequirement(const std::uint8_t pSoftwareVersion, const std::uint8_t pFirmwareVersion)
{
    return pFirmwareVersion == pSoftwareVersion;
}

/*!
 * \copybrief checkVersionRequirement(std::uint8_t, std::uint8_t)
 *
 * This is a shortcut for checkVersionRequirement(std::uint8_t, std::uint8_t)
 * being called with getModuleSoftwareVersion() and getModuleFirmwareVersion().
 *
 * \return If versions are compatible according to checkVersionRequirement(std::uint8_t, std::uint8_t).
 */
bool RegisterDriver::checkVersionRequirement()
{
    return checkVersionRequirement(getModuleSoftwareVersion(), getModuleFirmwareVersion());
}

//

/*!
 * \brief Read a byte sequence from a register address.
 *
 * Reads \p pRegSize bytes at register address \p pRegAddr via read().
 *
 * \throws std::runtime_error If read() fails or the number of received bytes differs from \p pRegSize.
 *
 * \param pRegAddr Module-local register address.
 * \param pRegSize Register size in bytes.
 * \return Read bytes.
 */
std::vector<std::uint8_t> RegisterDriver::getRegBytes(const std::uint32_t pRegAddr, const std::uint32_t pRegSize) const
{
    const std::vector<std::uint8_t> readBytes = read(pRegAddr, pRegSize);

    if (readBytes.size() != pRegSize)
        throw std::runtime_error("Read wrong number of bytes.");

    return readBytes;
}

/*!
 * \brief Write a byte sequence to a register address.
 *
 * Writes \p pData to register address \p pRegAddr via write().
 *
 * \throws std::runtime_error If write() fails.
 *
 * \param pRegAddr Module-local register address.
 * \param pData Byte sequence to be written.
 */
void RegisterDriver::setRegBytes(const std::uint32_t pRegAddr, const std::vector<std::uint8_t>& pData) const
{
    write(pRegAddr, pData);
}

//

/*!
 * \brief Read an integer value from a register address.
 *
 * Reads \c N full bytes at register address \p pRegAddr via read(), with \c N such that the contained integer
 * value at bit offset \p pRegOffs and with bit size \p pRegSize can be determined. This value will be returned.
 *
 * \throws std::runtime_error If read() fails or the number of received bytes differs from \c N.
 *
 * \param pRegAddr Module-local register address (in bytes).
 * \param pRegSize Register size in bits (i.e. bit length of stored value).
 * \param pRegOffs Register offset in bits (i.e. bit offset of stored value with respect to \p pRegAddr).
 * \return Read value.
 */
std::uint64_t RegisterDriver::getRegValue(const std::uint32_t pRegAddr, const std::uint32_t pRegSize, const std::uint32_t pRegOffs) const
{
    const std::uint32_t byteOffs = pRegOffs / 8;
    const std::uint32_t bitOffs = pRegOffs % 8;

    std::uint32_t readByteSize = (bitOffs + pRegSize) / 8;
    if ((bitOffs + pRegSize) % 8 > 0)
        ++readByteSize;

    const std::vector<std::uint8_t> readBytes = read(pRegAddr + byteOffs, readByteSize);

    if (readBytes.size() != readByteSize)
        throw std::runtime_error("Read wrong number of bytes.");

    if (readByteSize > 9)
    {
        throw std::runtime_error("Read size of register exceeds 64 bits. THIS SHOULD NEVER HAPPEN!");
    }
    else if (readByteSize == 9)
    {
        const std::uint8_t partialValLhs = readBytes[0];
        const std::uint64_t partialValRhs = Bytes::composeUInt64(std::span<const std::uint8_t, 8>(readBytes.begin()+1, 8), true);

        const std::size_t lhsBitCount = 8 - bitOffs;

        const std::uint64_t intermedVal = (static_cast<std::uint64_t>(static_cast<std::uint8_t>(partialValLhs << bitOffs)) << 56) |
                                     (static_cast<std::uint64_t>(partialValRhs >> lhsBitCount));

        return static_cast<std::uint64_t>(intermedVal >> static_cast<std::size_t>(64 - pRegSize));
    }
    else if (readByteSize == 8)
    {
        const std::uint64_t tmpVal = Bytes::composeUInt64(readBytes, true);
        return static_cast<std::uint64_t>(static_cast<std::uint64_t>(tmpVal << bitOffs) >> static_cast<std::size_t>(64 - pRegSize));
    }
    else if (readByteSize > 4)
    {
        const std::size_t paddingBytes = 8 - readByteSize;
        const std::size_t paddedBitOffs = 8*paddingBytes + bitOffs;

        std::vector<std::uint8_t> readBytesPadded(8, 0);
        std::copy(readBytes.begin(), readBytes.end(), readBytesPadded.begin()+paddingBytes);

        const std::uint64_t tmpVal = Bytes::composeUInt64(readBytesPadded, true);
        return static_cast<std::uint64_t>(static_cast<std::uint64_t>(tmpVal << paddedBitOffs) >> static_cast<std::size_t>(64 - pRegSize));
    }
    else if (readByteSize == 4)
    {
        const std::uint32_t tmpVal = Bytes::composeUInt32(readBytes, true);
        return static_cast<std::uint32_t>(static_cast<std::uint32_t>(tmpVal << bitOffs) >> static_cast<std::size_t>(32 - pRegSize));
    }
    else if (readByteSize == 3)
    {
        constexpr std::size_t paddingBytes = 1;
        const std::size_t paddedBitOffs = 8*paddingBytes + bitOffs;

        const std::vector<std::uint8_t> readBytesPadded {0, readBytes[0], readBytes[1], readBytes[2]};

        const std::uint32_t tmpVal = Bytes::composeUInt32(readBytesPadded, true);
        return static_cast<std::uint32_t>(static_cast<std::uint32_t>(tmpVal << paddedBitOffs) >> static_cast<std::size_t>(32 - pRegSize));
    }
    else if (readByteSize == 2)
    {
        const std::uint16_t tmpVal = Bytes::composeUInt16(readBytes, true);
        return static_cast<std::uint16_t>(static_cast<std::uint16_t>(tmpVal << bitOffs) >> static_cast<std::size_t>(16 - pRegSize));
    }
    else if (readByteSize == 1)
    {
        const std::uint8_t tmpVal = readBytes[0];
        return static_cast<std::uint8_t>(static_cast<std::uint8_t>(tmpVal << bitOffs) >> static_cast<std::size_t>(8 - pRegSize));
    }
    else
        throw std::runtime_error("Read size of register is zero. THIS SHOULD NEVER HAPPEN!");
}

/*!
 * \brief Write an integer value to a register address.
 *
 * If the register only covers \e full bytes (i.e. \p pRegSize and \p pRegOffs each a multiple of 8), writes the new value
 * \p pValue to the register at address \p pRegAddr, with bit offset \p pRegOffs and bit size \p pRegSize using write().
 *
 * Otherwise, first \e reads the \c N covered bytes using read() (similar to getRegValue()),
 * modifies only the bits in <tt>[pRegOffs, pRegOffs+pRegSize)</tt>, which represent the stored
 * value, and then writes back the partially modified byte sequence to \p pRegAddr using write().
 *
 * \throws std::runtime_error If the potential read() fails or the number of received bytes differs from \c N.
 * \throws std::runtime_error If write() fails.
 *
 * \param pRegAddr Module-local register address (in bytes).
 * \param pRegSize Register size in bits (i.e. bit length of stored value).
 * \param pRegOffs Register offset in bits (i.e. bit offset of stored value with respect to \p pRegAddr).
 * \param pValue Value to be written.
 */
void RegisterDriver::setRegValue(const std::uint32_t pRegAddr, const std::uint32_t pRegSize,
                                 const std::uint32_t pRegOffs, const std::uint64_t pValue) const
{
    const std::uint32_t byteOffs = pRegOffs / 8;
    const std::uint32_t bitOffs = pRegOffs % 8;

    std::uint32_t writeByteSize = (bitOffs + pRegSize) / 8;
    if ((bitOffs + pRegSize) % 8 > 0)
        ++writeByteSize;

    if (bitOffs == 0 && (pRegSize % 8) == 0)
    {
        if (writeByteSize > 8)
        {
            throw std::runtime_error("Write size of register without offset exceeds 8 bytes. THIS SHOULD NEVER HAPPEN!");
        }
        else if (writeByteSize == 8)
        {
            write(pRegAddr + byteOffs, Bytes::composeByteVec(true, static_cast<std::uint64_t>(pValue)));
        }
        else if (writeByteSize > 4)
        {
            const std::vector<std::uint8_t> writeBytes = Bytes::composeByteVec(true, static_cast<std::uint64_t>(pValue));
            std::vector<std::uint8_t> writeBytesTruncated(writeByteSize, 0);

            const std::size_t skipBytes = 8 - writeByteSize;
            std::copy(writeBytes.begin()+skipBytes, writeBytes.end(), writeBytesTruncated.begin());

            write(pRegAddr + byteOffs, writeBytesTruncated);
        }
        else if (writeByteSize == 4)
        {
            write(pRegAddr + byteOffs, Bytes::composeByteVec(true, static_cast<std::uint32_t>(pValue)));
        }
        else if (writeByteSize == 3)
        {
            const std::vector<std::uint8_t> writeBytes = Bytes::composeByteVec(true, static_cast<std::uint32_t>(pValue));
            const std::vector<std::uint8_t> writeBytesTruncated {writeBytes[1], writeBytes[2], writeBytes[3]};

            write(pRegAddr + byteOffs, writeBytesTruncated);
        }
        else if (writeByteSize == 2)
        {
            write(pRegAddr + byteOffs, Bytes::composeByteVec(true, static_cast<std::uint16_t>(pValue)));
        }
        else if (writeByteSize == 1)
        {
            write(pRegAddr + byteOffs, Bytes::composeByteVec(true, static_cast<std::uint8_t>(pValue)));
        }
        else
        {
            throw std::runtime_error("Write size of register is zero. THIS SHOULD NEVER HAPPEN!");
        }
    }
    else
    {
        std::vector<std::uint8_t> readBytes = read(pRegAddr + byteOffs, writeByteSize);

        if (readBytes.size() != writeByteSize)
            throw std::runtime_error("Read wrong number of bytes.");

        std::bitset<64> valueBitset(pValue);

        for (std::size_t i = 0; i < writeByteSize; ++i)
        {
            std::bitset<8> currentByte(readBytes[i]);

            if (writeByteSize == 1)
            {
                for (std::size_t j = bitOffs; j < bitOffs + pRegSize; ++j)
                    currentByte[7 - j] = valueBitset[pRegSize - 1 - (j - bitOffs)];
            }
            else if (i == 0)
            {
                for (std::size_t j = bitOffs; j < 8; ++j)
                    currentByte[7 - j] = valueBitset[pRegSize - 1 - (j - bitOffs)];
            }
            else if (i == writeByteSize - 1)
            {
                const std::size_t maxJ = 8 - (8*writeByteSize - bitOffs - pRegSize);    //This is in [1, 8]

                for (std::size_t j = 0; j < maxJ; ++j)
                    currentByte[7 - j] = valueBitset[maxJ - 1 - j];
            }
            else
            {
                for (std::size_t j = 0; j < 8; ++j)
                    currentByte[7 - j] = valueBitset[pRegSize - 1 - (8*i - bitOffs) - j];
            }

            readBytes[i] = static_cast<std::uint8_t>(currentByte.to_ulong());
        }

        write(pRegAddr + byteOffs, readBytes);
    }
}

//RegisterDriver::RegisterProxy

using RegisterProxy = RegisterDriver::RegisterProxy;

/*!
 * \brief Constructor.
 *
 * Binds the register access functions to the register \p pRegName of driver \p pRegDriver.
 *
 * \param pRegDriver The driver instance for the register to be controlled.
 * \param pRegName Name of the register.
 */
RegisterProxy::RegisterProxy(RegisterDriver& pRegDriver, std::string pRegName) :
    regDriver(pRegDriver),
    regName(std::move(pRegName))
{
}

//Public

/*!
 * \brief Write an integer value to the register.
 *
 * Writes \p pValue to the register via RegisterDriver::setValue().
 *
 * \throws std::invalid_argument See RegisterDriver::setValue().
 * \throws std::runtime_error See RegisterDriver::setValue().
 *
 * \param pValue Value to be written.
 * \return \p pValue.
 */
#ifdef CASIL_DOXYGEN    //Workaround for Doxygen getting confused by the added const
std::uint64_t RegisterProxy::operator=(/*const */std::uint64_t pValue) const
#else
std::uint64_t RegisterProxy::operator=(const std::uint64_t pValue) const
#endif
{
    regDriver.setValue(regName, pValue);
    return pValue;
}

/*!
 * \brief Write a byte sequence to the register.
 *
 * Writes \p pBytes to the register via RegisterDriver::setBytes().
 *
 * \throws std::invalid_argument See RegisterDriver::setBytes().
 * \throws std::runtime_error See RegisterDriver::setBytes().
 *
 * \param pBytes Byte sequence to be written.
 * \return \p pBytes.
 */
const std::vector<std::uint8_t>& RegisterProxy::operator=(const std::vector<std::uint8_t>& pBytes) const
{
    regDriver.setBytes(regName, pBytes);
    return pBytes;
}

//

/*!
 * \brief Read an integer value from the register.
 *
 * Reads the unsigned integer value stored by the register via RegisterDriver::getValue(std::string_view).
 *
 * \throws std::invalid_argument See RegisterDriver::getValue(std::string_view).
 * \throws std::runtime_error See RegisterDriver::getValue(std::string_view).
 *
 * \return Value stored in the register (or zero if write-only).
 */
RegisterProxy::operator std::uint64_t() const
{
    return regDriver.getValue(regName);
}

/*!
 * \fn RegisterProxy::operator std::vector<std::uint8_t>()
 * \brief Read a byte sequence from the register.
 *
 * Reads the byte sequence of the register via RegisterDriver::getBytes(std::string_view).
 *
 * \throws std::invalid_argument See RegisterDriver::getBytes(std::string_view).
 * \throws std::runtime_error See RegisterDriver::getBytes(std::string_view).
 *
 * \return Byte sequence stored in the register (or empty vector if write-only).
 */
RegisterProxy::operator std::vector<std::uint8_t>() const
{
    return regDriver.getBytes(regName);
}

//

/*!
 * \brief Read an integer or byte sequence from the register, according to its data type.
 *
 * Reads the value/sequence stored by the register via RegisterDriver::get().
 *
 * \throws std::invalid_argument See RegisterDriver::get().
 * \throws std::runtime_error See RegisterDriver::get().
 *
 * \return Integer value or byte sequence, depending on the \ref RegisterDescr::DataType "DataType" of the register.
 */
std::variant<std::uint64_t, std::vector<std::uint8_t>> RegisterProxy::get() const
{
    return regDriver.get(regName);
}

//

/*!
 * \brief "Trigger" the (write-only) register by writing configured default or zero.
 *
 * Triggers the register via RegisterDriver::trigger().
 *
 * \throws std::invalid_argument See RegisterDriver::trigger().
 * \throws std::runtime_error See RegisterDriver::trigger().
 */
void RegisterProxy::trigger() const
{
    regDriver.trigger(regName);
}
