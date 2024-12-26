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
*/

#include <casil/HL/muxeddriver.h>

#include <casil/bytes.h>

#include <stdexcept>
#include <utility>

using casil::Layers::HL::MuxedDriver;

/*!
 * \brief Constructor.
 *
 * Constructs Driver by forwarding \p pType, \p pName, \p pConfig and \p pRequiredConfig.
 *
 * Initializes the internal muxed interface instance (for required access to \ref Layers::TL "TL") from \p pInterface.
 *
 * Gets the mandatory "base_addr" value from \p pConfig (unsigned integer value), which must be the bus address of the
 * corresponding firmware module instance. This \e base \e address is used to offset fixed module addresses according to
 * the instance bus address when calling TL::MuxedInterface::read() / TL::MuxedInterface::write() / TL::MuxedInterface::query().
 * See also read() / write() / query() from this class.
 *
 * \throws std::runtime_error If "base_addr" is not an unsigned integer (max. 64 bit).
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 */
MuxedDriver::MuxedDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                         LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    Driver(std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig),
    interface(pInterface),
    baseAddr(config.getUInt("base_addr", 0x0u))
{
    if (!config.contains(LayerConfig::fromYAML("{base_addr: uint}"), true))
        throw std::runtime_error("Invalid or no base address (\"base_addr\") set for " + getSelfDescription() + ".");
}

//Public

/*!
 * \copybrief Driver::getData()
 *
 * Does nothing (override for specific drivers if needed).
 *
 * \param pSize Potentially number of bytes to get (implementation-defined).
 * \param pAddrOffs Potentially data offset as number of bytes (implementation-defined).
 * \return Empty vector.
 */
std::vector<std::uint8_t> MuxedDriver::getData(int, std::uint32_t)
{
    return {};  //(sic!)
}

/*!
 * \copybrief Driver::setData()
 *
 * Does nothing (override for specific drivers if needed).
 *
 * \copydetails Driver::setData()
 */
void MuxedDriver::setData(const std::vector<std::uint8_t>&, std::uint32_t)
{
    //(sic!)
}

/*!
 * \copybrief Driver::exec()
 *
 * Does nothing (override for specific drivers if needed).
 */
void MuxedDriver::exec()
{
    //(sic!)
}

/*!
 * \copybrief Driver::isDone()
 *
 * Does nothing (override for specific drivers if needed).
 *
 * \return False.
 */
bool MuxedDriver::isDone()
{
    return false;   //(sic!)
}

//Protected

/*!
 * \brief Read from the interface relative to the base address.
 *
 * Calls TL::MuxedInterface::read() with \p pAddr being offset by the module instance's base address
 * (component configuration parameter "base_addr").
 *
 * \throws std::runtime_error If TL::MuxedInterface::read() throws \c std::runtime_error.
 *
 * \param pAddr Module-local address.
 * \param pSize Number of bytes to read.
 * \return Read bytes.
 */
std::vector<std::uint8_t> MuxedDriver::read(const std::uint64_t pAddr, const int pSize) const
{
    try
    {
        return interface.read(baseAddr + pAddr, pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to read from interface (address: " + Bytes::formatHex(pAddr) +
                                 ", size: " + std::to_string(pSize) + "): " + exc.what());
    }
}

/*!
 * \brief Write to the interface relative to the base address.
 *
 * Calls TL::MuxedInterface::write() with \p pAddr being offset by the module instance's base address
 * (component configuration parameter "base_addr").
 *
 * \throws std::runtime_error If TL::MuxedInterface::write() throws \c std::runtime_error.
 *
 * \param pAddr Module-local address.
 * \param pData %Bytes to be written.
 */
void MuxedDriver::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) const
{
    try
    {
        interface.write(baseAddr + pAddr, pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to write to interface (address: " + Bytes::formatHex(pAddr) +
                                 ", data: " + Bytes::formatByteVec(pData) + "): " + exc.what());
    }
}

/*!
 * \brief Write a query to the interface and read the response, both relative to the base address.
 *
 * Calls TL::MuxedInterface::query() with \p pAddr being offset by the module instance's base address
 * (component configuration parameter "base_addr").
 *
 * \throws std::runtime_error If TL::MuxedInterface::query() throws \c std::runtime_error.
 *
 * \param pWriteAddr Module-local address to write to.
 * \param pReadAddr Module-local address to read from.
 * \param pData Query bytes to be written.
 * \param pSize Number of response bytes to read.
 * \return Read bytes.
 */
std::vector<std::uint8_t> MuxedDriver::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                             const std::vector<std::uint8_t>& pData, const int pSize) const
{
    try
    {
        return interface.query(baseAddr + pWriteAddr, baseAddr + pReadAddr, pData, pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Muxed driver \"" + name + "\" failed to query from interface (write address: " +
                                 Bytes::formatHex(pWriteAddr) + ", read address: " + Bytes::formatHex(pReadAddr) +
                                 ", data: " + Bytes::formatByteVec(pData) + ", size: " + std::to_string(pSize) + "): " + exc.what());
    }
}
