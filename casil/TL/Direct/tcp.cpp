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

#include <casil/TL/Direct/tcp.h>

#include <casil/logger.h>

#include <stdexcept>
#include <utility>

using casil::TL::TCP;

CASIL_REGISTER_INTERFACE_CPP(TCP)
CASIL_REGISTER_INTERFACE_ALIAS("Socket")

//

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

std::vector<std::uint8_t> TCP::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    return DirectInterface::query(pData, pSize);
}

//

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
