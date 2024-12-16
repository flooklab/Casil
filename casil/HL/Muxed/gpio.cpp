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

#include <casil/HL/Muxed/gpio.h>

#include <casil/bytes.h>
#include <casil/logger.h>

#include <boost/dynamic_bitset.hpp>

#include <bitset>
#include <utility>

using casil::HL::GPIO;

CASIL_REGISTER_DRIVER_CPP(GPIO)
CASIL_REGISTER_DRIVER_ALIAS("gpio")

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pName
 * \param pInterface
 * \param pConfig
 */
GPIO::GPIO(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, pConfig, LayerConfig::fromYAML("{size: uint}"), getRegisterDescrs(pConfig)),
    size(config.getUInt("size", 8)),
    ioBytes(((size - 1) / 8) + 1)
{
    if (size == 0)
        throw std::runtime_error("Invalid IO port count set for " + getSelfDescription() + ".");
}

//Public

/*!
 * \brief Get the \c INPUT register.
 *
 * \todo Detailed doc
 *
 * \param pSize
 * \param pAddrOffs
 * \return
 */
std::vector<std::uint8_t> GPIO::getData(int, std::uint32_t)
{
    return getBytes("INPUT");
}

/*!
 * \brief Set the \c OUTPUT register.
 *
 * \todo Detailed doc
 *
 * \param pData
 * \param pAddrOffs
 */
void GPIO::setData(const std::vector<std::uint8_t>& pData, std::uint32_t)
{
    setBytes("OUTPUT", pData);
}

//

/*!
 * \brief Get the number of IO bits.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::uint64_t GPIO::getSize() const
{
    return size;
}

//

/*!
 * \brief Get the \c OUTPUT_EN register.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::vector<std::uint8_t> GPIO::getOutputEn()
{
    return getBytes("OUTPUT_EN");
}

/*!
 * \brief Set the \c OUTPUT_EN register.
 *
 * \todo Detailed doc
 *
 * \param pEnable
 */
void GPIO::setOutputEn(const std::vector<std::uint8_t>& pEnable)
{
    setBytes("OUTPUT_EN", pEnable);
}

//

/*!
 * \brief Convert IO register bytes to a bitset.
 *
 * Converts a sequence of %GPIO's IO register bytes (such as returned by e.g. getOutputEn())
 * to a bitset that corresponds to the actual represented IO bits in the firmware module.
 *
 * See also Bytes::bitsetFromBytes().
 *
 * \throws std::invalid_argument If size of \p pBytes differs from ((getSize() - 1) / 8) + 1.
 *
 * \param pBytes The byte sequence to convert.
 * \return The IO bits.
 */
boost::dynamic_bitset<> GPIO::bitsetFromBytes(const std::vector<std::uint8_t>& pBytes) const
{
    if (pBytes.size() != ioBytes)
        throw std::invalid_argument("Wrong number of bytes for GPIO driver \"" + name + "\".");

    return Bytes::bitsetFromBytes(pBytes, size);
}

/*!
 * \brief Convert a bitset to IO register bytes.
 *
 * Converts a sequence of actual IO bits of/for the firmware module (i.e. with length equal to getSize())
 * to a byte sequence that corresponds to the full register bytes (as needed by e.g. setOutputEn()).
 *
 * See also Bytes::bytesFromBitset().
 *
 * \throws std::invalid_argument If bit size of \p pBits differs from getSize().
 *
 * \param pBits The bit sequence to convert.
 * \return The register bytes.
 */
std::vector<std::uint8_t> GPIO::bytesFromBitset(const boost::dynamic_bitset<>& pBits) const
{
    if (pBits.size() != size)
        throw std::invalid_argument("Wrong number of bits for GPIO driver \"" + name + "\".");

    return Bytes::bytesFromBitset(pBits, ioBytes);
}

//Private

/*!
 * \copybrief RegisterDriver::initModule()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool GPIO::initModule()
{
    if (config.contains(LayerConfig::fromYAML("{init: {output_en: string}}"), true))
    {
        Logger::logWarning("The \"init.output_en\" setting is unsupported but set for " + getSelfDescription() + ". " +
                           "Please use the register init setting \"init.OUTPUT_EN\" instead.");
    }

    return true;
}

//

/*!
 * \copybrief RegisterDriver::resetImpl()
 *
 * \todo Detailed doc
 */
void GPIO::resetImpl()
{
    setValue("RESET", 0);
}

//

/*!
 * \copybrief RegisterDriver::getModuleSoftwareVersion()
 *
 * \todo Detailed doc
 *
 * \return
 */
std::uint8_t GPIO::getModuleSoftwareVersion() const
{
    return requireFirmwareVersion;
}

/*!
 * \copybrief RegisterDriver::getModuleFirmwareVersion()
 *
 * \todo Detailed doc
 *
 * \return
 */
std::uint8_t GPIO::getModuleFirmwareVersion()
{
    return static_cast<std::uint8_t>(getValue("VERSION"));
}

//

/*!
 * \brief Generate the map of registers depending on the configured bit count.
 *
 * \todo Detailed doc
 *
 * \param pConfig
 * \return
 */
std::map<std::string, casil::HL::RegisterDescr, std::less<>> GPIO::getRegisterDescrs(const LayerConfig& pConfig)
{
    const std::uint32_t numIOBytes = ((pConfig.getUInt("size", 8) - 1) / 8) + 1;

    return {
        {"RESET",     {.type{DataType::Value},     .mode{AccessMode::WriteOnly}, .addr{0},                .size{8},          .offs{0}}},
        {"VERSION",   {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0},                .size{8},          .offs{0}}},
        {"INPUT",     {.type{DataType::ByteArray}, .mode{AccessMode::ReadOnly},  .addr{1},                .size{numIOBytes}, .offs{0}}},
        {"OUTPUT",    {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{1 + 1*numIOBytes}, .size{numIOBytes}, .offs{0}}},
        {"OUTPUT_EN", {.type{DataType::ByteArray}, .mode{AccessMode::ReadWrite}, .addr{1 + 2*numIOBytes}, .size{numIOBytes}, .offs{0}}}
    };
}
