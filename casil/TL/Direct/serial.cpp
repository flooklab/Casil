/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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

#include <casil/TL/Direct/serial.h>

#include <casil/asio.h>
#include <casil/TL/CommonImpl/serialportwrapper.h>

#include <boost/system/system_error.hpp>

#include <stdexcept>
#include <utility>

using casil::Layers::TL::Serial;

CASIL_REGISTER_INTERFACE_CPP(Serial)

//

/*!
 * \brief Constructor.
 *
 * Initializes the device name of the serial port to be opened from the mandatory "init.port" string in \p pConfig.
 *
 * Initializes the baud rate setting for the serial communication from the mandatory "init.baudrate" value (integer type) in \p pConfig.
 *
 * Initializes the termination sequence for non-sized read operations from the mandatory "init.read_termination" string in \p pConfig.
 *
 * Initializes the termination sequence for write operations from the optional "init.write_termination" string in \p pConfig or,
 * if not defined, to the same sequence as the read termination.
 *
 * \throws std::runtime_error If "init.port" is empty.
 * \throws std::runtime_error If "init.baudrate" is zero or negative.
 * \throws std::runtime_error If "init.read_termination" is not defined.
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
Serial::Serial(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                        "{init: {port: string, read_termination: string, baudrate: int}}")
                    ),
    port(config.getStr("init.port", "")),
    readTermination(config.getStr("init.read_termination", "\r\n")),
    writeTermination(config.getStr("init.write_termination", readTermination)),
    baudRate(config.getInt("init.baudrate", 9600)),
    serialPortWrapperPtr(std::make_unique<CommonImpl::SerialPortWrapper>(port, readTermination, writeTermination, baudRate))
{
    if (port == "")
        throw std::runtime_error("No serial port set for " + getSelfDescription() + ".");
    if (baudRate <= 0)
        throw std::runtime_error("Negative baud rate set for " + getSelfDescription() + ".");
}

/*!
 * \brief Default destructor.
 */
Serial::~Serial() = default;

//Public

/*!
 * \copybrief DirectInterface::read()
 *
 * Reads \p pSize bytes if \p pSize is positive and any number of bytes up
 * to (but excluding) the configured read termination if \p pSize is -1.
 * Other negative values return an empty sequence.
 *
 * \copydetails DirectInterface::read()
 */
std::vector<std::uint8_t> Serial::read(const int pSize)
{
    return serialPortWrapperPtr->read(pSize);
}

/*!
 * \copybrief DirectInterface::write()
 *
 * \throws std::runtime_error If the write fails.
 *
 * \copydetails DirectInterface::write()
 */
void Serial::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        serialPortWrapperPtr->write(pData);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to serial port \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief DirectInterface::query()
 *
 * \copydetails DirectInterface::query()
 */
std::vector<std::uint8_t> Serial::query(const std::vector<std::uint8_t>& pData, const int pSize)
{
    return DirectInterface::query(pData, pSize);
}

//

/*!
 * \copybrief DirectInterface::readBufferEmpty()
 *
 * \copydetails DirectInterface::readBufferEmpty()
 */
bool Serial::readBufferEmpty() const
{
    return serialPortWrapperPtr->readBufferEmpty();
}

/*!
 * \copybrief DirectInterface::clearReadBuffer()
 */
void Serial::clearReadBuffer()
{
    serialPortWrapperPtr->clearReadBuffer();
}

//Private

/*!
 * \copybrief DirectInterface::initImpl()
 *
 * Opens the serial port using the configured device name, sets the configured baud rate and starts
 * continuous polling to fill the read buffer with incoming data (see also CommonImpl::SerialPortWrapper).
 *
 * \note Requires IO context threads to be running already (see ASIO::ioContextThreadsRunning()).
 *
 * \return True if successful.
 */
bool Serial::initImpl()
{
    try
    {
        serialPortWrapperPtr->init();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not open serial port: ") + exc.what());
        return false;
    }

    return true;
}

/*!
 * \copybrief DirectInterface::closeImpl()
 *
 * Stops read buffer polling started by init() (see also CommonImpl::SerialPortWrapper) and closes the port.
 *
 * \return True if successful.
 */
bool Serial::closeImpl()
{
    try
    {
        serialPortWrapperPtr->close();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not close serial port: ") + exc.what());
        return false;
    }

    return true;
}
