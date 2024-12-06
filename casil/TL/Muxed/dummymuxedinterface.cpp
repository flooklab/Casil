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

#include <casil/TL/Muxed/dummymuxedinterface.h>

#include <casil/bytes.h>
#include <casil/logger.h>

#include <utility>

using casil::TL::DummyMuxedInterface;

CASIL_REGISTER_INTERFACE_CPP(DummyMuxedInterface)

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pName
 * \param pConfig
 */
DummyMuxedInterface::DummyMuxedInterface(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \copybrief MuxedInterface::read()
 *
 * \todo Detailed doc
 *
 * \param pAddr
 * \param pSize
 * \return
 */
std::vector<std::uint8_t> DummyMuxedInterface::read(const std::uint64_t pAddr, const int pSize)
{
    Logger::logDebug("Called read() for " + getSelfDescription() + " with arguments " +
                     "\"pAddr\" = " + Bytes::formatHex(pAddr) + ", " +
                     "\"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

/*!
 * \copybrief MuxedInterface::write()
 *
 * \todo Detailed doc
 *
 * \param pAddr
 * \param pData
 */
void DummyMuxedInterface::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    Logger::logDebug("Called write() for " + getSelfDescription() + " with arguments " +
                     "\"pAddr\" = " + Bytes::formatHex(pAddr) + ", " +
                     "\"pData\" = " + Bytes::formatByteVec(pData) + ".");
}

/*!
 * \copybrief MuxedInterface::query()
 *
 * \todo Detailed doc
 *
 * \param pWriteAddr
 * \param pReadAddr
 * \param pData
 * \param pSize
 * \return
 */
std::vector<std::uint8_t> DummyMuxedInterface::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                                     const std::vector<std::uint8_t>& pData, const int pSize)
{
    Logger::logDebug("Called query() for " + getSelfDescription() + " with arguments " +
                     "\"pWriteAddr\" = " + Bytes::formatHex(pWriteAddr) + ", " +
                     "\"pReadAddr\" = " + Bytes::formatHex(pReadAddr) + ", " +
                     "\"pData\" = " + Bytes::formatByteVec(pData) + ", " +
                     "\"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

//

/*!
 * \copybrief MuxedInterface::readBufferEmpty()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DummyMuxedInterface::readBufferEmpty() const
{
    Logger::logDebug("Called readBufferEmpty() for " + getSelfDescription() + ".");
    return true;
}

/*!
 * \copybrief MuxedInterface::clearReadBuffer()
 *
 * \todo Detailed doc
 */
void DummyMuxedInterface::clearReadBuffer()
{
    Logger::logDebug("Called clearReadBuffer() for " + getSelfDescription() + ".");
}

//Private

/*!
 * \copybrief MuxedInterface::initImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DummyMuxedInterface::initImpl()
{
    Logger::logDebug("Called initImpl() for " + getSelfDescription() + ".");
    return true;
}

/*!
 * \copybrief MuxedInterface::closeImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DummyMuxedInterface::closeImpl()
{
    Logger::logDebug("Called closeImpl() for " + getSelfDescription() + ".");
    return true;
}
