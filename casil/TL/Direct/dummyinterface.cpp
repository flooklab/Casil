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

#include <casil/TL/Direct/dummyinterface.h>

#include <casil/bytes.h>

#include <utility>

using casil::Layers::TL::DummyInterface;

CASIL_REGISTER_INTERFACE_CPP(DummyInterface)

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
DummyInterface::DummyInterface(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \copybrief DirectInterface::read()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pSize Number of bytes to read (ignored).
 * \return Empty vector.
 */
std::vector<std::uint8_t> DummyInterface::read(const int pSize)
{
    logger.logDebug("read() was called with argument \"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

/*!
 * \copybrief DirectInterface::write()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pData %Bytes to be written (ignored).
 */
void DummyInterface::write(const std::vector<std::uint8_t>& pData)
{
    logger.logDebug("write() was called with argument \"pData\" = " + Bytes::formatByteVec(pData) + ".");
}

/*!
 * \copybrief DirectInterface::query()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call and the passed arguments.
 *
 * \param pData Query bytes to be written (ignored).
 * \param pSize Number of response bytes to read (ignored).
 * \return Empty vector.
 */
std::vector<std::uint8_t> DummyInterface::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    logger.logDebug(std::string("query() was called with arguments ") +
                    "\"pData\" = " + Bytes::formatByteVec(pData) + ", " +
                    "\"pSize\" = " + std::to_string(pSize) + ".");
    return {};
}

//

/*!
 * \copybrief DirectInterface::readBufferEmpty()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyInterface::readBufferEmpty() const
{
    logger.logDebug("readBufferEmpty() was called.");
    return true;
}

/*!
 * \copybrief DirectInterface::clearReadBuffer()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 */
void DummyInterface::clearReadBuffer()
{
    logger.logDebug("clearReadBuffer() was called.");
}

//Private

/*!
 * \copybrief DirectInterface::initImpl()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyInterface::initImpl()
{
    logger.logDebug("initImpl() was called.");
    return true;
}

/*!
 * \copybrief DirectInterface::closeImpl()
 *
 * Does nothing except \ref Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyInterface::closeImpl()
{
    logger.logDebug("closeImpl() was called.");
    return true;
}
