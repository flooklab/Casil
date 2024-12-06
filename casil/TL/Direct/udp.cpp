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

#include <casil/TL/Direct/udp.h>

#include <casil/logger.h>

#include <stdexcept>
#include <utility>

using casil::TL::UDP;

CASIL_REGISTER_INTERFACE_CPP(UDP)

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pName
 * \param pConfig
 */
UDP::UDP(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                        "{init: {address: string, port: int}}")
                    ),
    hostName(config.getStr("init.address", "")),
    port(config.getInt("init.port", 1)),
    socketWrapper(hostName, port)
{
    if (hostName == "")
        throw std::runtime_error("No address/hostname set for " + getSelfDescription() + ".");
    if (port <= 0 || port > 65535)
        throw std::runtime_error("Invalid port number set for " + getSelfDescription() + ".");
}

//Public

/*!
 * \copybrief DirectInterface::read()
 *
 * \todo Detailed doc
 *
 * \param pSize
 * \return
 */
std::vector<std::uint8_t> UDP::read(const int pSize)
{
    (void)pSize;

    try
    {
        return socketWrapper.read();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not read from UDP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::write()
 *
 * \todo Detailed doc
 *
 * \param pData
 */
void UDP::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        socketWrapper.write(pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to UDP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::query()
 *
 * \todo Detailed doc
 *
 * \param pData
 * \param pSize
 * \return
 */
std::vector<std::uint8_t> UDP::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    return DirectInterface::query(pData, pSize);
}

//

/*!
 * \copybrief DirectInterface::readBufferEmpty()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool UDP::readBufferEmpty() const
{
    try
    {
        return socketWrapper.readBufferEmpty();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not check read buffer size of UDP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::clearReadBuffer()
 *
 * \todo Detailed doc
 */
void UDP::clearReadBuffer()
{
    try
    {
        socketWrapper.clearReadBuffer();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear read buffer of UDP socket \"" + name + "\": " + exc.what());
    }
}

//Private

/*!
 * \copybrief DirectInterface::initImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool UDP::initImpl()
{
    try
    {
        socketWrapper.init();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not connect socket of " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    return true;
}

/*!
 * \copybrief DirectInterface::closeImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool UDP::closeImpl()
{
    try
    {
        socketWrapper.close();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not close socket connection of " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    return true;
}
