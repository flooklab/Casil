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

#include <casil/HL/registerdriver.h>

#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/logger.h>

#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <bitset>
#include <cctype>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <utility>

using casil::HL::RegisterDriver;

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
 *
 * \todo Throws for all the failure modes
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 * \param pRegisters
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

        //Initialize register written value cache
        if (regDescr.mode != AccessMode::ReadOnly)
            registerWrittenCache[regName] = std::monostate{};

        //Collect manually overridden default values from "init" map of configuration YAML

        initValues[regName] = std::monostate{};

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

        //Initialize register proxies
        registerProxies.emplace(std::piecewise_construct, std::forward_as_tuple(regName), std::forward_as_tuple(*this, regName));
    }
}

//Public

/*!
 * \brief Access a register via the proxy class.
 *
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
 * \todo Detailed doc
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
 * \todo Detailed doc
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
                {
                    Logger::logWarning("Byte sequence read from register \"" + std::string(pRegName) + "\" of " +
                                       getSelfDescription() + " differs from cached one.");
                }
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
 * \brief Write data to a byte array register.
 *
 * \todo Detailed doc
 *
 * \param pRegName
 * \param pData
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
                {
                    Logger::logWarning("Value read from register \"" + std::string(pRegName) + "\" of " +
                                       getSelfDescription() + " differs from cached value.");
                }
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
 * \brief Write a value to a value register.
 *
 * \todo Detailed doc
 *
 * \param pRegName
 * \param pValue
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \param pValue
 */
void RegisterDriver::set(const std::string_view pRegName, const std::uint64_t pValue)
{
    setValue(pRegName, pValue);
}

/*!
 * \brief Write data to a byte array register.
 *
 * \todo Detailed doc
 *
 * \param pRegName
 * \param pBytes
 */
void RegisterDriver::set(const std::string_view pRegName, const std::vector<std::uint8_t>& pBytes)
{
    setBytes(pRegName, pBytes);
}

//

/*!
 * \brief "Trigger" a write-only register by writing configured default or zero.
 *
 * \todo Detailed doc
 *
 * \param pRegName
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
 * \todo Detailed doc
 *
 * \return
 */
bool RegisterDriver::initImpl()
{
    try
    {
        reset();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not reset " + getSelfDescription() + ": " + exc.what());
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
        Logger::logError("Could not determine firmware version of FPGA module used for " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    if (!checkVersionRequirement(softwareVersion, firmwareVersion))
    {
        Logger::logError("FPGA module used for " + getSelfDescription() + " reports incompatible firmware version: " +
                         std::to_string(firmwareVersion) + "; driver software version is: " + std::to_string(softwareVersion) + ".");
        return false;
    }

    try
    {
        applyDefaults();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not write default register states for " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    if (!initModule())
        return false;

    Logger::logSuccess("Initialized " + getSelfDescription() + ": Using FPGA module at base address " +
                       Bytes::formatHex(baseAddr) + " with firmware version " + std::to_string(firmwareVersion) +
                       " (driver version: " + std::to_string(softwareVersion) + ").");

    return true;
}

/*!
 * \copybrief MuxedDriver::closeImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool RegisterDriver::closeImpl()
{
    if (!closeModule())
        return false;

    return true;
}

//

/*!
 * \brief Perform module-specific initialization steps.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool RegisterDriver::initModule()
{
    return true;
}

/*!
 * \brief Perform module-specific closing steps.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool RegisterDriver::closeModule()
{
    return true;
}

//

/*!
 * \brief Check if software version is compatible with firmware version.
 *
 * \todo Detailed doc
 *
 * \param pSoftwareVersion
 * \param pFirmwareVersion
 * \return
 */
bool RegisterDriver::checkVersionRequirement(const std::uint8_t pSoftwareVersion, const std::uint8_t pFirmwareVersion)
{
    return pFirmwareVersion == pSoftwareVersion;
}

/*!
 * \copybrief checkVersionRequirement(std::uint8_t, std::uint8_t)
 *
 * \todo Detailed doc
 *
 * \return
 */
bool RegisterDriver::checkVersionRequirement()
{
    return checkVersionRequirement(getModuleSoftwareVersion(), getModuleFirmwareVersion());
}

//

/*!
 * \brief Read a byte sequence from a register address.
 *
 * \todo Detailed doc
 *
 * \param pRegAddr
 * \param pRegSize
 * \return
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
 * \todo Detailed doc
 *
 * \param pRegAddr
 * \param pData
 */
void RegisterDriver::setRegBytes(const std::uint32_t pRegAddr, const std::vector<std::uint8_t>& pData) const
{
    write(pRegAddr, pData);
}

//

/*!
 * \brief Read an integer value from a register address.
 *
 * \todo Detailed doc
 *
 * \param pRegAddr
 * \param pRegSize
 * \param pRegOffs
 * \return
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
 * \todo Detailed doc
 *
 * \param pRegAddr
 * \param pRegSize
 * \param pRegOffs
 * \param pValue
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
 * \todo Detailed doc
 *
 * \param pRegDriver
 * \param pRegName
 */
RegisterProxy::RegisterProxy(RegisterDriver& pRegDriver, std::string pRegName) :
    regDriver(pRegDriver),
    regName(std::move(pRegName))
{
}

/*!
 * \brief Write an integer value to the register.
 *
 * \todo Detailed doc
 *
 * \param pValue
 * \return
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
 * \todo Detailed doc
 *
 * \param pBytes
 * \return
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
 * \todo Detailed doc
 */
RegisterProxy::operator std::uint64_t() const
{
    return regDriver.getValue(regName);
}

/*!
 * \brief Read a byte sequence from the register.
 *
 * \todo Detailed doc
 */
RegisterProxy::operator std::vector<std::uint8_t>() const
{
    return regDriver.getBytes(regName);
}

//

/*!
 * \brief Read an integer or byte sequence from the register, according to its data type.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::variant<std::uint64_t, std::vector<std::uint8_t>> RegisterProxy::get() const
{
    return regDriver.get(regName);
}

//

/*!
 * \brief "Trigger" the (write-only) register by writing configured default or zero.
 *
 * \todo Detailed doc
 */
void RegisterProxy::trigger() const
{
    regDriver.trigger(regName);
}
