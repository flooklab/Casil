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

using casil::Layers::TL::UDP;

CASIL_REGISTER_INTERFACE_CPP(UDP)

//

/*!
 * \brief Constructor.
 *
 * Initializes the host name to connect to from the mandatory "init.address" string in \p pConfig.
 *
 * Initializes the network port for the communication from the mandatory "init.port" value (integer type) in \p pConfig.
 *
 * \throws std::runtime_error If "init.address" is empty.
 * \throws std::runtime_error If "init.port" is out of range (must be in <tt>(0, 65535]</tt>).
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
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
 * Receives and returns a single incoming datagram, ignoring \p pSize.
 *
 * \throws std::runtime_error If the read fails.
 *
 * \param pSize Ignored.
 * \return Read bytes.
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
 * Sends a single datagram with payload \p pData.
 *
 * \throws std::runtime_error If the write fails.
 *
 * \copydetails DirectInterface::write()
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
 * Clears the read buffer if not empty, writes one datagram with \p pData, waits for a potential
 * query delay (see Interface::Interface()) and reads one datagram (see also write() and read()).
 *
 * \throws std::runtime_error If readBufferEmpty(), clearReadBuffer(), write() or read() throw \c std::runtime_error.
 *
 * \param pData Query bytes to be written.
 * \param pSize Ignored.
 * \return Read bytes.
 */
std::vector<std::uint8_t> UDP::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    return DirectInterface::query(pData, pSize);
}

//

/*!
 * \copybrief DirectInterface::readBufferEmpty()
 *
 * \throws std::runtime_error If checking the buffer size fails.
 *
 * \copydetails DirectInterface::readBufferEmpty()
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
 * \throws std::runtime_error If clearing the buffer fails.
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
 * Resolves the configured host name and connects the socket to this endpoint via the configured port.
 *
 * \note Requires IO context threads to be running already (see ASIO::ioContextThreadsRunning()).
 *
 * \return True if successful.
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
 * Disconnects the socket.
 *
 * \return True if successful.
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
