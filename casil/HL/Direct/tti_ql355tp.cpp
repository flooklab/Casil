/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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
//
//  This file has been directly adapted from the corresponding code of
//  the basil software, which itself is covered by the following license:
//
//      Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn
//
//      All rights reserved.
//
//      Redistribution and use in source and binary forms, with or without
//      modification, are permitted provided that the following conditions are
//      met:
//
//       *  Redistributions of source code must retain the above copyright notice,
//          this list of conditions and the following disclaimer.
//
//       *  Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the distribution.
//
//       *  Neither the name of the copyright holder nor the names of its
//          contributors may be used to endorse or promote products derived from
//          this software without specific prior written permission.
//
//      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
//      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
//      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#include <casil/HL/Direct/tti_ql355tp.h>

#include <casil/bytes.h>

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>

using casil::Layers::HL::TTiQL355TP;

CASIL_REGISTER_DRIVER_CPP(TTiQL355TP)
CASIL_REGISTER_DRIVER_ALIAS("tti_ql355tp")

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
TTiQL355TP::TTiQL355TP(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \brief Re-initialize underlying interface instance.
 *
 * See TL::Interface::close() and TL::Interface::init().
 *
 * \return True if successful.
 */
bool TTiQL355TP::reInitIntf() const
{
    return (interface.close() && interface.init());
}

//

/*!
 * \brief Execute an arbitrary write command.
 *
 * Writes \p pCmd to the device.
 *
 * \throws std::runtime_error If writing to the interface fails.
 *
 * \param pCmd Raw %SCPI command as a string.
 */
void TTiQL355TP::writeCmd(const std::string& pCmd) const
{
    interface.write(Bytes::byteVecFromStr(pCmd));
}

/*!
 * \brief Execute an arbitrary query command.
 *
 * Writes \p pCmd to the device (see writeCmd()), waits 0.1 seconds and then reads
 * and returns the response from the interface (relying on its read termination).
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pCmd Raw %SCPI command as a string.
 * \return Response from the device as a string.
 */
std::string TTiQL355TP::queryCmd(const std::string& pCmd) const
{
    writeCmd(pCmd);
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    return Bytes::strFromByteVec(interface.read());
}

//

/*!
 * \brief Get the %SCPI device identifier.
 *
 * Queries the %SCPI device identifier from the connected device using the \c *IDN? command.
 *
 * Note: This should of course be "QL355TP" (as it is checked for init() anyway), but who knows.
 *
 * \throws std::runtime_error If queryCmd() fails and throws \c std::runtime_error.
 *
 * \return The obtained %SCPI device identifier.
 */
std::string TTiQL355TP::getDeviceIdentifier() const
{
    return queryCmd("*IDN?");
}

//

/*!
 * \brief Enable/disable one or all output channels.
 *
 * Enables the output of channel \p pChannel if \p pOn is true and disables it otherwise.
 * This works for channels 1, 2 and 3 (the \c AUX channel), as well as \e all channels at once if \p pChannel is -1.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2, 3, -1}).
 * \throws std::runtime_error If writeCmd() fails and throws \c std::runtime_error.
 *
 * \param pOn Switch the channel \e on or \e off.
 * \param pChannel Channel number out of {1, 2, 3}, or -1 for switching \e all channels.
 */
void TTiQL355TP::setEnable(const bool pOn, const int pChannel) const
{
    if (pChannel == -1)
        writeCmd(pOn ? "OPALL 1" : "OPALL 0");
    else if (pChannel == 1 || pChannel == 2 || pChannel == 3)
        writeCmd("OP" + std::to_string(pChannel) + (pOn ? " 1" : " 0"));
    else
        throw std::invalid_argument("Invalid channel number for " + getSelfDescription() + ".");
}

/*!
 * \brief Get the measured output voltage of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If queryCmd() fails and throws \c std::runtime_error.
 * \throws std::runtime_error If the query response has an unexpected format or cannot be parsed/converted for other reasons.
 *
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 * \return The measured voltage in volts.
 */
double TTiQL355TP::getVoltage(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for voltage query of " + getSelfDescription() + ".");

    const std::string valStr = queryCmd("V" + std::to_string(pChannel) + "O?");

    if (!valStr.ends_with("V"))
        throw std::runtime_error("Voltage query response has unexpected format for " + getSelfDescription() + ".");

    try
    {
        return std::stod(valStr.substr(0, valStr.size()-1));
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Could not convert voltage query response for " + getSelfDescription() + ".");
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Converted voltage query response is out of range for " + getSelfDescription() + ".");
    }
}

/*!
 * \brief Get the set output voltage of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If queryCmd() fails and throws \c std::runtime_error.
 * \throws std::runtime_error If the query response has an unexpected format or cannot be parsed/converted for other reasons.
 *
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 * \return The configured set voltage in volts.
 */
double TTiQL355TP::getSetVoltage(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for set voltage query of " + getSelfDescription() + ".");

    const std::string valStr = queryCmd("V" + std::to_string(pChannel) + "?");

    if (!valStr.starts_with("V" + std::to_string(pChannel) + " "))
        throw std::runtime_error("Set voltage query response has unexpected format for " + getSelfDescription() + ".");

    try
    {
        return std::stod(valStr.substr(3));
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Could not convert set voltage query response for " + getSelfDescription() + ".");
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Converted set voltage query response is out of range for " + getSelfDescription() + ".");
    }
}

/*!
 * \brief Set the output voltage of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If writeCmd() fails and throws \c std::runtime_error.
 *
 * \param pValue The desired voltage in volts.
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 */
void TTiQL355TP::setVoltage(const double pValue, const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for setting voltage of " + getSelfDescription() + ".");

    writeCmd("V" + std::to_string(pChannel) + " " + std::format("{: <-#E}", pValue));
}

/*!
 * \brief Get the measured output current of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If queryCmd() fails and throws \c std::runtime_error.
 * \throws std::runtime_error If the query response has an unexpected format or cannot be parsed/converted for other reasons.
 *
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 * \return The measured current in amperes.
 */
double TTiQL355TP::getCurrent(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for current query of " + getSelfDescription() + ".");

    const std::string valStr = queryCmd("I" + std::to_string(pChannel) + "O?");

    if (!valStr.ends_with("A"))
        throw std::runtime_error("Current query response has unexpected format for " + getSelfDescription() + ".");

    try
    {
        return std::stod(valStr.substr(0, valStr.size()-1));
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Could not convert current query response for " + getSelfDescription() + ".");
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Converted current query response is out of range for " + getSelfDescription() + ".");
    }
}

/*!
 * \brief Get the set current limit of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If queryCmd() fails and throws \c std::runtime_error.
 * \throws std::runtime_error If the query response has an unexpected format or cannot be parsed/converted for other reasons.
 *
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 * \return The configured current limit in amperes.
 */
double TTiQL355TP::getCurrentLimit(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for current limit query of " + getSelfDescription() + ".");

    const std::string valStr = queryCmd("I" + std::to_string(pChannel) + "?");

    if (!valStr.starts_with("I" + std::to_string(pChannel) + " "))
        throw std::runtime_error("Current limit query response has unexpected format for " + getSelfDescription() + ".");

    try
    {
        return std::stod(valStr.substr(3));
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Could not convert current limit query response for " + getSelfDescription() + ".");
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Converted current limit query response is out of range for " + getSelfDescription() + ".");
    }
}

/*!
 * \brief Set the current limit of a channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2}).
 * \throws std::runtime_error If writeCmd() fails and throws \c std::runtime_error.
 *
 * \param pValue The desired current limit in amperes.
 * \param pChannel Channel number (either 1 or 2, no \c AUX channel).
 */
void TTiQL355TP::setCurrentLimit(const double pValue, const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2))
        throw std::invalid_argument("Invalid channel number for setting current limit of " + getSelfDescription() + ".");

    writeCmd("I" + std::to_string(pChannel) + " " + std::format("{: <-#E}", pValue));
}

//

/*!
 * \brief Reset the OVP and OCP trips.
 *
 * Resets the over-voltage protection (OVP) and over-current protection (OCP) trips.
 *
 * \throws std::runtime_error If writeCmd() fails and throws \c std::runtime_error.
 */
void TTiQL355TP::resetTrip() const
{
    writeCmd("TRIPRST");
}

//Private

/*!
 * \copybrief DirectDriver::initImpl()
 *
 * Queries the %SCPI device identifier from the device using the standard command \c *IDN?
 * and compares it to the expected result "QL355TP". The function returns false on mismatch.
 *
 * \return True if successful.
 */
bool TTiQL355TP::initImpl()
{
    try
    {
        const std::string ident = getDeviceIdentifier();

        if (ident != "QL355TP")
        {
            logger.logError("Read wrong device identifier (expected: \"QL355TP\"; actual: \"" + ident + "\").");
            return false;
        }
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not initialize: ") + exc.what());
        return false;
    }

    return true;
}

/*!
 * \copybrief DirectDriver::closeImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool TTiQL355TP::closeImpl()
{
    return true;
}
