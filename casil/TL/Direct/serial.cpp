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

#include <casil/TL/Direct/serial.h>

#include <casil/asio.h>
#include <casil/logger.h>

#include <boost/system/system_error.hpp>

#include <stdexcept>
#include <utility>

using casil::TL::Serial;

CASIL_REGISTER_INTERFACE_CPP(Serial)

//

Serial::Serial(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                        "{init: {port: string, read_termination: string, baudrate: int}}")
                    ),
    port(config.getStr("init.port", "")),
    readTermination(config.getStr("init.read_termination", "\r\n")),
    writeTermination(config.getStr("init.write_termination", readTermination)),
    baudRate(config.getInt("init.baudrate", 9600)),
    serialPortWrapper(port, readTermination, writeTermination, baudRate)
{
    if (port == "")
        throw std::runtime_error("No serial port set for " + getSelfDescription() + ".");
    if (baudRate <= 0)
        throw std::runtime_error("Negative baud rate set for " + getSelfDescription() + ".");
}

//Public

std::vector<std::uint8_t> Serial::read(const int pSize)
{
    return serialPortWrapper.read(pSize);
}

void Serial::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        serialPortWrapper.write(pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to serial port \"" + name + "\": " + exc.what());
    }
}

std::vector<std::uint8_t> Serial::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    return DirectInterface::query(pData, pSize);
}

//

bool Serial::readBufferEmpty() const
{
    return serialPortWrapper.readBufferEmpty();
}

void Serial::clearReadBuffer()
{
    serialPortWrapper.clearReadBuffer();
}

//Private

bool Serial::initImpl()
{
    try
    {
        serialPortWrapper.init();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not initialize " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    return true;
}

bool Serial::closeImpl()
{
    try
    {
        serialPortWrapper.close();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not close " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    return true;
}
