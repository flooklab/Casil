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

#include <casil/HL/Direct/sensirion_ekh4.h>

#include <casil/bytes.h>

#include <chrono>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <thread>
#include <utility>

using casil::Layers::HL::SensirionEKH4;

CASIL_REGISTER_DRIVER_CPP(SensirionEKH4)
CASIL_REGISTER_DRIVER_ALIAS("sensirion_ekh4")

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
SensirionEKH4::SensirionEKH4(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \brief Get the temperature values from all four sensors.
 *
 * Determines and returns all four temperatures at once. Individual channel values will be set to \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \interface \sa getValues() \endinternal
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \return Temperatures in degrees Celsius (or \c std::nullopt) as {T_Sens1, T_Sens2, T_Sens3, T_Sens4}.
 */
std::array<std::optional<double>, 4> SensirionEKH4::getTemperatures() const
{
    return getValues("\x7e\x47\x00\xb8\x7e");
}

/*!
 * \brief Get the humidity values from all four sensors.
 *
 * Determines and returns all four humidities at once. Individual channel values will be set to \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \interface \sa getValues() \endinternal
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \return Humidities in ??? (TODO which unit?) (or \c std::nullopt) as {H_Sens1, H_Sens2, H_Sens3, H_Sens4}.
 */
std::array<std::optional<double>, 4> SensirionEKH4::getHumidities() const
{
    return getValues("\x7e\x46\x00\xb9\x7e");
}

/*!
 * \brief Get the dew point values from all four sensors.
 *
 * Determines and returns all four dew points at once. Individual channel values will be set to \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \interface \sa getValues() \endinternal
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \return Dew points in degrees Celsius (or \c std::nullopt) as {T_Sens1, T_Sens2, T_Sens3, T_Sens4}.
 */
std::array<std::optional<double>, 4> SensirionEKH4::getDewPoints() const
{
    return getValues("\x7e\x48\x00\xb7\x7e");
}

//

/*!
 * \brief Get the temperature value of one of the sensors.
 *
 * Determines and returns the temperature value of channel \p pChannel, which will be \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2, 3, 4}).
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pChannel Channel number out of {1, 2, 3, 4}.
 * \return Temperature in degrees Celsius, or \c std::nullopt if data could not be interpreted or no sensor connected.
 */
std::optional<double> SensirionEKH4::getTemperature(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2 || pChannel == 3 || pChannel == 4))
        throw std::invalid_argument("Invalid channel number for " + getSelfDescription() + ".");

    return getTemperatures()[pChannel-1];
}

/*!
 * \brief Get the humidity value of one of the sensors.
 *
 * Determines and returns the humidity value of channel \p pChannel, which will be \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2, 3, 4}).
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pChannel Channel number out of {1, 2, 3, 4}.
 * \return Humidity in ??? (TODO which unit?), or \c std::nullopt if data could not be interpreted or no sensor connected.
 */
std::optional<double> SensirionEKH4::getHumidity(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2 || pChannel == 3 || pChannel == 4))
        throw std::invalid_argument("Invalid channel number for " + getSelfDescription() + ".");

    return getHumidities()[pChannel-1];
}

/*!
 * \brief Get the dew point value of one of the sensors.
 *
 * Determines and returns the dew point value of channel \p pChannel, which will be \c std::nullopt
 * if either the received data cannot be properly interpreted or no sensor is connected to that channel.
 *
 * \throws std::invalid_argument If \p pChannel is invalid (not in {1, 2, 3, 4}).
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pChannel Channel number out of {1, 2, 3, 4}.
 * \return Dew point in degrees Celsius, or \c std::nullopt if data could not be interpreted or no sensor connected.
 */
std::optional<double> SensirionEKH4::getDewPoint(const int pChannel) const
{
    if (!(pChannel == 1 || pChannel == 2 || pChannel == 3 || pChannel == 4))
        throw std::invalid_argument("Invalid channel number for " + getSelfDescription() + ".");

    return getDewPoints()[pChannel-1];
}

//Private

/*!
 * \brief Write a command to the device.
 *
 * Writes \p pCmd to the device.
 *
 * \throws std::runtime_error If writing to the interface fails.
 *
 * \param pCmd The command.
 */
void SensirionEKH4::write(const std::string_view pCmd) const
{
    interface.write(Bytes::byteVecFromStr(pCmd));
}

/*!
 * \brief Query data from the device.
 *
 * Writes \p pCmd to the device (see write()), waits 0.1 seconds and then reads up to 1024 bytes from the interface
 * (depending on the received data). The expected data word is extracted from the received sequence and then returned.
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pCmd The command.
 * \return Data word from the received device response.
 */
std::vector<std::uint8_t> SensirionEKH4::query(const std::string_view pCmd) const
{
    write(pCmd);
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    std::vector<std::uint8_t> word;
    bool dataFlag = false;

    for (int i = 0; i < 1024; ++i)
    {
        const auto vec = interface.read(1);
        if (vec.size() != 1)
            throw std::runtime_error("Readback during sensor query has wrong size for " + getSelfDescription() + ".");

        const std::uint8_t tByte = vec[0];

        if (tByte == 0x7Eu)     //Delimiter
        {
            if (!dataFlag)          //Data word comes next
                dataFlag = true;
            else                    //Data word finished
                break;
        }
        else if (dataFlag)
            word.push_back(tByte);
    }

    return word;
}

//

/*!
 * \brief Query data for all four sensors and interpret them as individual measurement values.
 *
 * Queries raw sensor data for command \p pCmd (see query()). For the three different commands that are used by this
 * SensirionEKH4 class (requesting temperature, humidity or dew point) a certain data format is expected/assumed,
 * which is decoded by this function into four floating-point numbers that represent the corresponding measurement
 * results as requested by \p pCmd. Note that the used protocol is not well understood and hence some assumptions on
 * the data format might not always hold, which will sometimes lead to parsing errors for some or even all of the channels.
 * In these cases (and if no sensor is connected for the channel) the affected channel values will be set to \c std::nullopt.
 *
 * Some further notes on the received data format:
 * - The first two bytes of the data word are unknown (e.g. \c 0x4608, \c 0x4708 or \c 0x4808)
 * - The last byte seems to be some kind of checksum maybe
 * - In between there should normally be 8 bytes (i.e. two per channel)
 * - However, sometimes it is more; two cases can be distinguished here:
 *   1. Some kind of common error code \c 0x7D31 plus an additional byte;
 *      in this case, the affected channel is set to \c std::nullopt,
 *      the extra byte is skipped and the other channels are interpreted as usual
 *   2. Varying amount of extra bytes with no recognizable error code;
 *      in this case, nothing can be reconstructed and all channels are set to \c std::nullopt.
 * - Apparently, case 1 is reproducible for temperatures between about <tt>43.53 °C</tt> and <tt>46.1 °C</tt>
 *
 * \throws std::runtime_error If writing to or reading from the interface fails.
 *
 * \param pCmd The query command to write.
 * \return Values (or \c std::nullopt) as requested by \p pCmd (e.g. temperatures, ...) as {v_Sens1, v_Sens2, v_Sens3, v_Sens4}.
 */
std::array<std::optional<double>, 4> SensirionEKH4::getValues(const std::string_view pCmd) const
{
    std::array<std::optional<double>, 4> retVal;

    const std::vector<std::uint8_t> rawData = query(pCmd);

    if (rawData.size() < 11 || rawData.size() > 15)
    {
        logger.logWarning("Query yielded invalid/incompatible data: All returned values are unset.");
        return retVal;
    }

    const std::span<const std::uint8_t, std::dynamic_extent> trimmedData(rawData.begin()+2, rawData.size()-3);

    std::size_t dataPos = 0;

    for (int i = 0; i < 4; ++i)
    {
        if (dataPos+1 >= trimmedData.size())
        {
            logger.logWarning("Query yielded invalid/incompatible data: All returned values are unset.");
            return retVal;
        }

        const std::uint8_t msb = trimmedData[dataPos];
        const std::uint8_t lsb = trimmedData[dataPos+1];

        if (msb == 0x7D && lsb == 0x31)
            dataPos += 3;   //On error just proceed to next sensor, additionally skipping one expected extra byte
        else
        {
            dataPos += 2;   //Proceed to next sensor in next iteration

            //No sensor connected, do not set value
            if (msb == 0x7F && lsb == 0xFF)
                continue;

            retVal[i] = static_cast<double>(static_cast<std::int16_t>(Bytes::composeUInt16({msb, lsb}))) / 100.0;
        }
    }

    return retVal;
}

//

/*!
 * \copybrief DirectDriver::initImpl()
 *
 * Sets the sensor update interval to 1 hertz.
 *
 * \return True if successful.
 */
bool SensirionEKH4::initImpl()
{
    try
    {
        write("\x7e\x23\x02\x00\x01\x31\x02\x01\x0c\x25\x01\x0e\x26\x01\x03\x3a\x7e");  //Set update interval to 1 Hz
        (void)interface.read(1024);                                                     //Clear buffer
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
bool SensirionEKH4::closeImpl()
{
    return true;
}
