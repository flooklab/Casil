/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2026 M. Frohne
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
#include <casil/auxil.h>
#include <casil/logger.h>
#include <casil/TL/CommonImpl/serialportwrapper.h>

#include <boost/system/system_error.hpp>

#include <algorithm>
#include <array>
#include <optional>
#include <stdexcept>
#include <utility>

namespace
{

using casil::LayerConfig;
using casil::Layers::TL::CommonImpl::SerialPortWrapper;
using casil::Logger;

/*
 * This is a helper function for Serial::Serial().
 *
 * Checks if 'pParity' contains a single character that corresponds to a valid parity setting and throws std::runtime_error otherwise.
 * Available options are "N" (no parity), "O" (odd parity), "E" (even parity).
 *
 * Returns the requested parity setting from 'pParity' as the corresponding enum value for SerialPortWrapper.
 */
SerialPortWrapper::PortParity parseParity(const std::string& pParity)
{
    if (pParity.size() != 1 || !std::ranges::contains(std::array<char, 5>{'N', 'O', 'E', 'M', 'S'}, pParity.front()))
        throw std::runtime_error("Invalid parity option set: Must be one of {\"N\" (none), \"O\" (odd), \"E\" (even)}.");

    if (!std::ranges::contains(std::array<char, 3>{'N', 'O', 'E'}, pParity.front()))
        throw std::runtime_error("Unsupported parity option set: \"mark\" and \"space\" parity are not available.");

    using PortParity = SerialPortWrapper::PortParity;

    switch (pParity.front())
    {
        case 'N':
            return PortParity::None;
        case 'O':
            return PortParity::Odd;
        case 'E':
            return PortParity::Even;
        default:
            throw std::runtime_error("Invalid parity code. THIS SHOULD NEVER HAPPEN!");
    }
}

/*
 * This is a helper function for Serial::Serial().
 *
 * Checks if 'pStopBits' equals a valid stop bits setting and throws std::runtime_error otherwise.
 * Available options are "1" (one stop bit), "1.5" (one and a half stop bits), "2" (two stop bits).
 *
 * Returns the requested stop bits setting from 'pStopBits' as the corresponding enum value for SerialPortWrapper.
 */
SerialPortWrapper::PortStopBits parseStopBits(const std::string& pStopBits)
{
    using PortStopBits = SerialPortWrapper::PortStopBits;

    if (pStopBits == "1")
        return PortStopBits::One;
    else if (pStopBits == "1.5")
        return PortStopBits::OnePointFive;
    else if (pStopBits == "2")
        return PortStopBits::Two;
    else
        throw std::runtime_error("Invalid stop bits option set: Must be one of {\"1\", \"1.5\", \"2\"}.");
}

/*
 * This is a helper function for Serial::Serial().
 *
 * Checks if 'pFlowControl' contains a single character that corresponds to a valid flow control setting and throws
 * std::runtime_error otherwise. Available options are "N" (no flow control), "S" (software flow control), "H" (hardware flow control).
 *
 * Returns the requested flow control setting from 'pFlowControl' as the corresponding enum value for SerialPortWrapper.
 */
SerialPortWrapper::PortFlowControl parseFlowControl(const std::string& pFlowControl)
{
    //This check is pretty much redundant with the also used one in the below check function, but let's keep it anyway
    if (pFlowControl.size() != 1 || !std::ranges::contains(std::array<char, 3>{'N', 'S', 'H'}, pFlowControl.front()))
        throw std::runtime_error("Invalid flow control option set: Must be one of {\"N\" (none), \"S\" (software), \"H\" (hardware)}.");

    using PortFlowControl = SerialPortWrapper::PortFlowControl;

    switch (pFlowControl.front())
    {
        case 'N':
            return PortFlowControl::None;
        case 'S':
            return PortFlowControl::Software;
        case 'H':
            return PortFlowControl::Hardware;
        default:
            throw std::runtime_error("Invalid flow control code. THIS SHOULD NEVER HAPPEN!");
    }
}

/*
 * This is a helper function for Serial::Serial().
 *
 * Checks if potential combination of flow control options are all valid and compatible and throws std::runtime_error otherwise.
 * \e Either 'pFlowCtrl' must be set \e or a combination of the pair 'pXonXoff' and/or 'pRtsCts'. DSR/DTR is unsupported and hence
 * 'pDsrDtr' (if set) is only allowed to be false. 'pXonXoff' and 'pRtsCts' must not both be true at the same time.
 *
 * Returns the requested flow control setting from 'pFlowCtrl' (or alternatively from 'pXonXoff' and 'pRtsCts')
 * as a string as required by the "init.flow_ctrl" option (and as expected by parseFlowControl() above).
 * If none of the options is set (all strings empty), "N" will be returned (no flow control).
 */
std::string checkFlowControl(std::string&& pFlowCtrl, const std::string& pXonXoff, const std::string& pRtsCts, const std::string& pDsrDtr)
{
    if (pDsrDtr != "")
    {
        try
        {
            if (LayerConfig::fromYAML("{val: \"" + pDsrDtr + "\"}").getBool("val", true))
                throw std::runtime_error("");
        }
        catch (const std::runtime_error&)
        {
            throw std::runtime_error("Invalid flow control option set: DSR/DTR is unsupported.");
        }

        //Unsupported option used but set to false, hence just issue a warning
        Logger::logWarning("DSR/DTR flow control is unsupported.");
    }

    if (pXonXoff != "" || pRtsCts != "")
        Logger::logWarning("Flow control options \"xonxoff\"/\"rtscts\"/\"dsrdtr\" are deprecated, use \"flow_ctrl\" instead.");

    bool xonXoff = false;
    bool rtsCts = false;

    if (pXonXoff != "")
    {
        try
        {
            std::optional<bool> tVal = LayerConfig::fromYAML("{val: \"" + pXonXoff + "\"}").getBoolOpt("val");
            if (tVal.has_value())
                xonXoff = tVal.value();
            else
                throw std::runtime_error("");
        }
        catch (const std::runtime_error&)
        {
            throw std::runtime_error("Could not parse serial port flow control option \"xonxoff\".");
        }
    }

    if (pRtsCts != "")
    {
        try
        {
            std::optional<bool> tVal = LayerConfig::fromYAML("{val: \"" + pRtsCts + "\"}").getBoolOpt("val");
            if (tVal.has_value())
                rtsCts = tVal.value();
            else
                throw std::runtime_error("");
        }
        catch (const std::runtime_error&)
        {
            throw std::runtime_error("Could not parse serial port flow control option \"rtscts\".");
        }
    }

    if (pFlowCtrl != "" && (pXonXoff != "" || pRtsCts != ""))
        throw std::runtime_error("Conflicting flow control options set, use \"flow_ctrl\" only.");

    if (pFlowCtrl == "")
    {
        if (xonXoff && rtsCts)
            throw std::runtime_error("Contradictory flow control options set: Cannot use both hardware and software flow control .");
        else if (xonXoff)
            return "S";
        else if (rtsCts)
            return "H";
        else
            return "N";
    }
    else
    {
        if (pFlowCtrl.size() != 1 || !std::ranges::contains(std::array<char, 3>{'N', 'S', 'H'}, pFlowCtrl.front()))
            throw std::runtime_error("Invalid flow control option set: Must be one of {\"N\" (none), \"S\" (software), \"H\" (hardware)}.");

        return std::move(pFlowCtrl);
    }
}

} // namespace

using casil::Layers::TL::Serial;

CASIL_REGISTER_INTERFACE_CPP(Serial)

//

/*!
 * \brief Constructor.
 *
 * Initializes the device name of the serial port to be opened from the mandatory "init.port" string in \p pConfig.
 *
 * Initializes the baud rate setting for the serial communication from the mandatory "init.baudrate" value
 * (unsigned integer type) in \p pConfig.
 *
 * Initializes the character size setting for the serial communication from the optional "init.bytesize"
 * value (unsigned integer type, default: 8) in \p pConfig. Supported values are 5, 6, 7 and 8.
 *
 * Initializes the parity setting for the serial communication from the optional "init.parity" value (string type, default: "N") in
 * \p pConfig. Available options are "N" (no parity), "O" (odd parity), "E" (even parity). Neither \e mark nor \e space parity are supported.
 *
 * Initializes the stop bit setting for the serial communication from the optional "init.stopbits" value (\e string type, default: "1")
 * in \p pConfig. Available options are (all <em>verbatim</em>!) "1" (one stop bit), "1.5" (one and a half stop bits), "2" (two stop bits).
 *
 * Initializes the flow control setting for the serial communication from the optional "init.flow_ctrl" value (string type, default: "N")
 * in \p pConfig. Available options are "N" (no flow control), "S" (software flow control), "H" (hardware flow control). Additionally,
 * for backwards compatibility, the deprecated values "init.xonxoff" (bool type, default: false) "init.rtscts" (bool type, default: false)
 * and "init.dsrdtr" (bool type, default: false) will be parsed and used instead if "init.flow_ctrl" is not set.
 * Note, however, that "init.dsrdtr" must actually not be set to true because DSR/DTR flow control is unsupported.
 *
 * Initializes the termination sequence for non-sized read operations from the mandatory "init.read_termination" string in \p pConfig.
 *
 * Initializes the termination sequence for write operations from the optional "init.write_termination" string in \p pConfig or,
 * if not defined, to the same sequence as the read termination.
 *
 * Initializes the overall/maximum timeout for read operations (see read()) from the optional "init.timeout" value
 * in \p pConfig (floating-point value in seconds, default: 0.0 (i.e. no timeout)).
 *
 * Initializes the "inter-character" timeout for read operations (see read()) from the optional "init.inter_byte_timeout" value
 * in \p pConfig (floating-point value in seconds, default: 0.0 (i.e. no timeout)). This specific timeout gets reset every time
 * that new partial data arrives (before data is complete according to requested amount of bytes or termination).
 * It can be combined with the regular maximum timeout above.
 *
 * Initializes the timeout for write operations (see write()) from the optional "init.write_timeout" value
 * in \p pConfig (floating-point value in seconds, default: 0.0 (i.e. no timeout)).
 *
 * \throws std::runtime_error If "init.port" is empty.
 * \throws std::runtime_error If "init.baudrate" is zero.
 * \throws std::runtime_error If "init.bytesize" is not in <tt>{5, 6, 7, 8}</tt>.
 * \throws std::runtime_error If "init.parity" is not in <tt>{'N', 'O', 'E'}</tt>.
 * \throws std::runtime_error If "init.stopbits" is not in <tt>{"1", "1.5", "2"}</tt>.
 * \throws std::runtime_error If "init.flow_ctrl" is not in <tt>{"N", "S", "H"}</tt>.
 * \throws std::runtime_error If "init.xonxoff", "init.rtscts" or "init.dsrdtr" can not be parsed as boolean.
 * \throws std::runtime_error If "init.dsrdtr" is defined and not false.
 * \throws std::runtime_error If "init.flow_ctrl" \e and one of "init.xonxoff" or "init.rtscts" are set.
 * \throws std::runtime_error If "init.xonxoff" (software flow control) and "init.rtscts" (hardware flow control) are both true.
 * \throws std::runtime_error If "init.read_termination" is not defined.
 * \throws std::runtime_error For negative timeout values ("init.timeout", "init.inter_byte_timeout", "init.write_timeout").
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
Serial::Serial(std::string pName, LayerConfig pConfig) :
    DirectInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                        "{init: {port: string, baudrate: uint, read_termination: string}}")
                    ),
    port(config.getStr("init.port", "")),
    readTermination(config.getStr("init.read_termination", "")),
    writeTermination(config.getStr("init.write_termination", readTermination)),
    baudRate(config.getUInt("init.baudrate", 0)),
    characterSize(config.getUInt("init.bytesize", 8)),
    parity(config.getStr("init.parity", "N")),
    stopBits(config.getStr("init.stopbits", "1")),
    flowControl(::checkFlowControl(config.getStr("init.flow_ctrl", ""), config.getStr("init.xonxoff", ""),
                                   config.getStr("init.rtscts", ""), config.getStr("init.dsrdtr", ""))),
    readTimeoutSecs(config.getDbl("init.timeout", 0.0)),
    readInterCharTimeoutSecs(config.getDbl("init.inter_byte_timeout", 0.0)),
    writeTimeoutSecs(config.getDbl("init.write_timeout", 0.0)),
    readTimeout(Auxil::getChronoMilliSecs(readTimeoutSecs)),
    readInterCharTimeout(Auxil::getChronoMilliSecs(readInterCharTimeoutSecs)),
    writeTimeout(Auxil::getChronoMilliSecs(writeTimeoutSecs)),
    serialPortWrapperPtr(std::make_unique<CommonImpl::SerialPortWrapper>(port, readTermination, writeTermination, baudRate, characterSize,
                                                                         ::parseParity(parity), ::parseStopBits(stopBits),
                                                                         ::parseFlowControl(flowControl)))
{
    if (port == "")
        throw std::runtime_error("No serial port set for " + getSelfDescription() + ".");
    if (baudRate == 0)
        throw std::runtime_error("Baud rate set to zero for " + getSelfDescription() + ".");
    if (characterSize < 5 || characterSize > 8)
        throw std::runtime_error("Invalid character/byte size set for " + getSelfDescription() + " (must be one of {5, 6, 7, 8}).");
    if (readTimeoutSecs < 0.0)
        throw std::runtime_error("Negative read timeout set for " + getSelfDescription() + ".");
    if (readInterCharTimeoutSecs < 0.0)
        throw std::runtime_error("Negative inter-character read timeout set for " + getSelfDescription() + ".");
    if (writeTimeoutSecs < 0.0)
        throw std::runtime_error("Negative write timeout set for " + getSelfDescription() + ".");
}

/*!
 * \brief Default destructor.
 */
Serial::~Serial() = default;

//Public

/*!
 * \copybrief DirectInterface::read()
 *
 * Reads \p pSize bytes if \p pSize is positive and any number of bytes up to (but excluding) the configured read termination if \p pSize
 * is -1. Other negative values return an empty sequence. Uses the timeouts from the component configuration, if set (see Serial()).
 * Note that in case of a timeout or other errors the returned data might be incomplete or contain part of the read termination.
 * If a specific (i.e. positive) \p pSize is requested, the data will, however, be filled with trailing zeros to match \p pSize.
 *
 * \internal See also CommonImpl::SerialPortWrapper::read(). \endinternal
 *
 * \copydetails DirectInterface::read()
 */
std::vector<std::uint8_t> Serial::read(const int pSize)
{
    return serialPortWrapperPtr->read(pSize, readTimeout, readInterCharTimeout);
}

/*!
 * \copybrief DirectInterface::write()
 *
 * Uses the write timeout from the component configuration, if set (see Serial()).
 *
 * \internal See also CommonImpl::SerialPortWrapper::write(). \endinternal
 *
 * \throws std::runtime_error If the write fails or the timeout is reached before completion.
 *
 * \copydetails DirectInterface::write()
 */
void Serial::write(const std::vector<std::uint8_t>& pData)
{
    try
    {
        serialPortWrapperPtr->write(pData, writeTimeout);
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
