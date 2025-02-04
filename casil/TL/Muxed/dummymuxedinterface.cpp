/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#include <utility>

using casil::Layers::TL::DummyMuxedInterface;

CASIL_REGISTER_INTERFACE_CPP(DummyMuxedInterface)

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
DummyMuxedInterface::DummyMuxedInterface(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \copybrief MuxedInterface::read()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pAddr Bus address (ignored).
 * \param pSize Number of bytes to read (ignored).
 * \return Empty vector.
 */
std::vector<std::uint8_t> DummyMuxedInterface::read(const std::uint64_t pAddr, const int pSize)
{
    logger.logDebug(std::string("read() was called with arguments ") +
                    "\"pAddr\" = " + Bytes::formatHex(pAddr) + ", " +
                    "\"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

/*!
 * \copybrief MuxedInterface::write()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pAddr Bus address (ignored).
 * \param pData %Bytes to be written (ignored).
 */
void DummyMuxedInterface::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    logger.logDebug(std::string("write() was called with arguments ") +
                    "\"pAddr\" = " + Bytes::formatHex(pAddr) + ", " +
                    "\"pData\" = " + Bytes::formatByteVec(pData) + ".");
}

/*!
 * \copybrief MuxedInterface::query()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pWriteAddr Bus address to write to (ignored).
 * \param pReadAddr Bus address to read from (ignored).
 * \param pData Query bytes to be written (ignored).
 * \param pSize Number of response bytes to read (ignored).
 * \return Empty vector.
 */
std::vector<std::uint8_t> DummyMuxedInterface::query(const std::uint64_t pWriteAddr, const std::uint64_t pReadAddr,
                                                     const std::vector<std::uint8_t>& pData, const int pSize)
{
    logger.logDebug(std::string("query() was called with arguments ") +
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
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyMuxedInterface::readBufferEmpty() const
{
    logger.logDebug("readBufferEmpty() was called.");
    return true;
}

/*!
 * \copybrief MuxedInterface::clearReadBuffer()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 */
void DummyMuxedInterface::clearReadBuffer()
{
    logger.logDebug("clearReadBuffer() was called.");
}

//Private

/*!
 * \copybrief MuxedInterface::initImpl()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyMuxedInterface::initImpl()
{
    logger.logDebug("initImpl() was called.");
    return true;
}

/*!
 * \copybrief MuxedInterface::closeImpl()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyMuxedInterface::closeImpl()
{
    logger.logDebug("closeImpl() was called.");
    return true;
}
