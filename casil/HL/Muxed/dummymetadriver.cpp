/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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
*/

#include <casil/HL/Muxed/dummymetadriver.h>

#include <casil/bytes.h>

#include <utility>

using casil::Layers::HL::DummyMetaDriver;

CASIL_REGISTER_META_DRIVER_CPP(DummyMetaDriver)

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pBackendDriver %Driver instance to be used as backend driver.
 * \param pConfig Component configuration.
 */
DummyMetaDriver::DummyMetaDriver(std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig) :
    MetaDriver(typeName, std::move(pName), pBackendDriver, std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \copybrief MetaDriver::getData()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \param pSize Number of bytes to get (ignored).
 * \param pAddrOffs Data offset as number of bytes (ignored).
 * \return Empty vector.
 */
std::vector<std::uint8_t> DummyMetaDriver::getData(const int pSize, const std::uint32_t pAddrOffs)
{
    logger.logDebug(std::string("getData() was called with arguments ") +
                    "\"pSize\" = " + std::to_string(pSize) + ", " +
                    "\"pAddrOffs\" = " + Bytes::formatHex(pAddrOffs) + ".");
    return {};
}

/*!
 * \copybrief MetaDriver::setData()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \param pData Data to be set as byte sequence (ignored).
 * \param pAddrOffs Data offset as number of bytes (ignored).
 */
void DummyMetaDriver::setData(const std::vector<std::uint8_t>& pData, const std::uint32_t pAddrOffs)
{
    logger.logDebug(std::string("setData() was called with arguments ") +
                    "\"pData\" = " + Bytes::formatByteVec(pData) + ", " +
                    "\"pAddrOffs\" = " + Bytes::formatHex(pAddrOffs) + ".");
}

/*!
 * \copybrief MetaDriver::exec()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 */
void DummyMetaDriver::exec()
{
    logger.logDebug("exec() was called.");
}

/*!
 * \copybrief MetaDriver::isDone()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return False.
 */
bool DummyMetaDriver::isDone()
{
    logger.logDebug("isDone() was called.");
    return false;
}

//Private

/*!
 * \copybrief MetaDriver::initImpl()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyMetaDriver::initImpl()
{
    logger.logDebug("initImpl() was called.");
    return true;
}

/*!
 * \copybrief MetaDriver::closeImpl()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyMetaDriver::closeImpl()
{
    logger.logDebug("closeImpl() was called.");
    return true;
}
