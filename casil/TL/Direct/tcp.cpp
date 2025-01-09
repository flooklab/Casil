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

#include <casil/TL/Direct/tcp.h>

#include <casil/logger.h>

#include <stdexcept>
#include <utility>

using casil::Layers::TL::TCP;

CASIL_REGISTER_INTERFACE_CPP(TCP)
CASIL_REGISTER_INTERFACE_ALIAS("Socket")

//

/*!
 * \brief Constructor.
 *
 * Initializes the host name to connect to from the mandatory "init.address" string in \p pConfig.
 *
 * Initializes the network port for the connection from the mandatory "init.port" value (integer type) in \p pConfig.
 *
 * Initializes the termination sequence for non-sized read operations from the mandatory "init.read_termination" string in \p pConfig.
 *
 * Initializes the termination sequence for write operations from the optional "init.write_termination" string in \p pConfig or,
 * if not defined, to the same sequence as the read termination.
 *
 * \throws std::runtime_error If "init.address" is empty.
 * \throws std::runtime_error If "init.port" is out of range (must be in <tt>(0, 65535]</tt>).
 * \throws std::runtime_error If "init.read_termination" is not defined.
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
TCP::TCP(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                        "{init: {address: string, port: int, read_termination: string}}")
                    ),
    hostName(config.getStr("init.address", "")),
    port(config.getInt("init.port", 1)),
    readTermination(config.getStr("init.read_termination", "\r\n")),
    writeTermination(config.getStr("init.write_termination", readTermination)),
    socketWrapper(hostName, port, readTermination, writeTermination)
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
 * Reads \p pSize bytes if \p pSize is positive and any number of bytes up
 * to (but excluding) the configured read termination if \p pSize is -1.
 * Other negative values return an empty sequence.
 *
 * \throws std::runtime_error If the read fails.
 *
 * \copydetails DirectInterface::read()
 */
std::vector<std::uint8_t> TCP::read(const int pSize)
{
    try
    {
        return socketWrapper.read(pSize);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not read from TCP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::write()
 *
 * \throws std::runtime_error If the write fails.
 *
 * \copydetails DirectInterface::write()
 */
void TCP::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        socketWrapper.write(pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to TCP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::query()
 *
 * \copydetails DirectInterface::query()
 */
std::vector<std::uint8_t> TCP::query(const std::vector<std::uint8_t>& pData, const int pSize)
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
bool TCP::readBufferEmpty() const
{
    try
    {
        return socketWrapper.readBufferEmpty();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not check read buffer size of TCP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::clearReadBuffer()
 *
 * \throws std::runtime_error If clearing the buffer fails.
 */
void TCP::clearReadBuffer()
{
    try
    {
        socketWrapper.clearReadBuffer();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear read buffer of TCP socket \"" + name + "\": " + exc.what());
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
bool TCP::initImpl()
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
bool TCP::closeImpl()
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
