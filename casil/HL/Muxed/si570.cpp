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

#include <casil/HL/Muxed/si570.h>

#include <casil/bytes.h>
#include <casil/HL/Muxed/i2c.h>

#include <cmath>
#include <ranges>
#include <stdexcept>
#include <utility>

using casil::Layers::HL::Si570;

CASIL_REGISTER_META_DRIVER_CPP(Si570)
CASIL_REGISTER_META_DRIVER_ALIAS("si570")

//

/*!
 * \brief Constructor.
 *
 * Gets the mandatory "init.frequency" value from \p pConfig (floating-point value in megahertz (MHz)),
 * which defines the initial reference frequency to set during init().
 *
 * \throws std::bad_cast If \p pBackendDriver is not HL::I2C.
 * \throws std::runtime_error If the %I2C transaction memory of \p pBackendDriver is smaller than \e seven bytes.
 * \throws std::runtime_error If reading the %I2C transaction memory size register fails.
 * \throws std::runtime_error If "init.frequency" is not defined, set to zero or is negative.
 *
 * \param pName Component instance name.
 * \param pBackendDriver %Driver instance to be used as backend driver.
 * \param pConfig Component configuration.
 */
Si570::Si570(std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig) :
    MetaDriver(typeName, std::move(pName), pBackendDriver, std::move(pConfig), LayerConfig::fromYAML("{init: {frequency: double}}")),
    i2cDrv(dynamic_cast<I2C&>(backendDriver)),  //Possible exception will be caught by macro-registered factory generator
    reg("", *this, LayerConfig::fromYAML("{size: 48, fields: ["
                                            "{name: RFREQ, offset: 37, size: 38},"
                                            "{name: N1, offset: 44, size: 7},"
                                            "{name: HS_DIV, offset: 47, size: 3}"
                                         "]}")),    //Using this as driver because need to pass one, but must not use related functionality
    initFrequency(config.getDbl("frequency", 0.0))
{
    if (i2cDrv.memSize() < 7)
    {
        throw std::runtime_error("Transaction memory of backend driver \"" + i2cDrv.getName() + "\" "
                                 "too small for " + getSelfDescription() + ".");
    }
    if (initFrequency == 0.0 || initFrequency < 0.0)
        throw std::runtime_error("Invalid frequency set for " + getSelfDescription() + ".");
}

//Public

/*!
 * \brief Reset the device.
 *
 * Performs a reset sequence for the %Si570 device.
 *
 * \throws std::runtime_error If any of the %I2C transactions fail.
 * \throws std::runtime_error If the readback from the %I2C read transaction has the wrong size for some reason.
 */
void Si570::reset()
{
    i2cDrv.sendWrite(0xBAu, Bytes::makeByteVec(135));
    const auto recall = i2cDrv.sendRead(0xBAu, 1);
    if (recall.size() != 1)
        throw std::runtime_error("Readback during reset sequence has wrong size for " + getSelfDescription() + ".");
    i2cDrv.sendWrite(0xBAu, {135, static_cast<std::uint8_t>(recall[0] | 0b1u)});
}

//

/*!
 * \brief Change reference frequency of %Si570 device.
 *
 * Resets the %Si570 (see reset()) and configures its registers \c HS_DIV, \c N1 and \c RFREQ
 * \internal (see readRegisters() / modifyRegisters()) \endinternal in order to set the reference frequency to \p pFreqMHz.
 *
 * \throws std::invalid_argument If \p pFreqMHz is invalid or out of range because no working values for \c HS_DIV and \c N1 could be found.
 * \throws std::runtime_error If any of the %I2C transactions fail.
 * \throws std::runtime_error If the readback from any of the %I2C read transactions has the wrong size for some reason.
 *
 * \param pFreqMHz New reference frequency in megahertz (MHz).
 */
void Si570::changeFrequency(const double pFreqMHz)
{
    if (pFreqMHz <= 0.0)
        throw std::invalid_argument("Invalid reference frequency for " + getSelfDescription() + ".");

    reset();

    const auto vals = readRegisters();

    std::uint8_t hsDiv = std::get<0>(vals);
    std::uint8_t n1 = std::get<1>(vals);
    const std::uint64_t rFreq = std::get<2>(vals);

    static constexpr double f0 = 156.25;
    const double fXtal = (f0 * hsDiv * n1) / (static_cast<double>(rFreq) / std::pow(2, 28));

    double newFDCO = pFreqMHz * hsDiv * n1;

    if (newFDCO < 4850.0 || newFDCO > 5670.0)
    {
        logger.logDebug("Large frequency change for Si570. Recalculating HS_DIV and N1...");

        bool foundNewVals = false;

        static constexpr std::array<std::uint8_t, 6> hsDivAvail = {11, 9, 7, 6, 5, 4};

        for (const auto hsVal : hsDivAvail)
        {
            for (const std::uint8_t nVal : std::views::iota(0, 65) | std::views::transform(             //Generate {1, 2, 4, 6, ..., 128}
                     [](const std::uint8_t pX){ if (pX == 0) { return 1; } else { return 2*pX; } }))
            {
                const auto fDCO = pFreqMHz * 1e6 * hsVal * nVal;

                if (fDCO >= 4.85e9 && fDCO <= 5.67e9)   //Range defined by manufacturer
                {
                    hsDiv = hsVal;
                    n1 = nVal;
                    foundNewVals = true;
                }
            }

            if (foundNewVals)
                break;
        }

        if (!foundNewVals)  //Correct HS_DIV and N1 were not found
        {
            throw std::invalid_argument("Requested reference frequency is out of range for " + getSelfDescription() + ": "
                                        "Could not find matching/working values of HS_DIV and N1.");
        }

        newFDCO = pFreqMHz * hsDiv * n1;
    }

    const double newRFreqFreq = newFDCO / fXtal;
    const auto newRFreq = static_cast<std::uint64_t>(newRFreqFreq * std::pow(2, 28));

    modifyRegisters(hsDiv, n1, newRFreq);

    logger.logInfo("Changed Si570 reference frequency to " + std::to_string(newFDCO / (hsDiv * n1)) + " MHz.");
}

//Private

/*!
 * \brief Read relevant register values from %Si570 device.
 *
 * Reads the current values of the %Si570 registers \c HS_DIV, \c N1 and \c RFREQ.
 *
 * \throws std::runtime_error If any of the %I2C transactions fail.
 * \throws std::runtime_error If the readback from the %I2C read transaction has the wrong size for some reason.
 *
 * \return Register values <tt>{HS_DIV, N1, RFREQ}</tt> as a tuple.
 */
std::tuple<std::uint8_t, std::uint8_t, std::uint64_t> Si570::readRegisters()
{
    i2cDrv.sendWrite(0xBAu, Bytes::makeByteVec(7));
    const auto regVal = i2cDrv.sendRead(0xBAu, 6);
    if (regVal.size() != 6)
        throw std::runtime_error("Readback during register reading has wrong size for " + getSelfDescription() + ".");

    const std::uint8_t hsDiv = ((regVal[0] & 0xE0u) >> 5) + 4;
    const std::uint8_t n1 = (static_cast<std::uint8_t>((regVal[0] & 0x1Fu) << 2) | static_cast<std::uint8_t>((regVal[1] & 0xC0u) >> 6)) + 1;
    const std::uint64_t rFreq = static_cast<std::uint64_t>((static_cast<std::uint64_t>(regVal[1] & 0x3Fu) << 32)) |
                                static_cast<std::uint64_t>(regVal[2] << 24) |
                                static_cast<std::uint64_t>(regVal[3] << 16) |
                                static_cast<std::uint64_t>(regVal[4] << 8) |
                                static_cast<std::uint64_t>(regVal[5]);

    return std::make_tuple(hsDiv, n1, rFreq);
}

/*!
 * \brief Write relevant register values to %Si570 device.
 *
 * Writes new values to the %Si570 registers \c HS_DIV, \c N1 and \c RFREQ.
 *
 * \throws std::runtime_error If any of the %I2C transactions fail.
 * \throws std::runtime_error If the readback from the %I2C read transaction has the wrong size for some reason.
 *
 * \param pHSDiv Value for the \c HS_DIV register.
 * \param pN1 Value for the \c N1 register.
 * \param pRFreq Value for the \c RFREQ register.
 */
void Si570::modifyRegisters(const std::uint8_t pHSDiv, const std::uint8_t pN1, const std::uint64_t pRFreq)
{
    i2cDrv.sendWrite(0xBAu, Bytes::makeByteVec(137));
    const auto dcoFreeze = i2cDrv.sendRead(0xBAu, 1);

    i2cDrv.sendWrite(0xBAu, Bytes::makeByteVec(135));
    const auto newFreqFlag = i2cDrv.sendRead(0xBAu, 1);

    if (dcoFreeze.size() != 1 || newFreqFlag.size() != 1)
        throw std::runtime_error("Readback during register modification has wrong size for " + getSelfDescription() + ".");

    //Freeze the DCO
    i2cDrv.sendWrite(0xBAu, {137, static_cast<std::uint8_t>(dcoFreeze[0] | 0b00010000u)});

    //Write the new frequency configuration
    reg["HS_DIV"] = pHSDiv - 4;
    reg["N1"] = pN1 - 1;
    reg["RFREQ"] = pRFreq;
    std::vector<std::uint8_t> freqVec{7};
    Bytes::appendToByteVec(freqVec, reg.toBytes());
    i2cDrv.sendWrite(0xBAu, freqVec);

    //Unfreeze the DCO
    i2cDrv.sendWrite(0xBAu, {137, static_cast<std::uint8_t>(dcoFreeze[0] & 0b00001111u)});

    //Assert the NewFreq bit
    i2cDrv.sendWrite(0xBAu, {135, static_cast<std::uint8_t>(newFreqFlag[0] | 0b01000000u)});
}

//

/*!
 * \copybrief MetaDriver::initImpl()
 *
 * Applies the "init.frequency" value from the component configuration (see Si570()) as the reference frequency (see changeFrequency()).
 *
 * \return True if successful.
 */
bool Si570::initImpl()
{
    try
    {
        changeFrequency(initFrequency);
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not initialize: ") + exc.what());
        return false;
    }

    return true;
}

/*!
 * \copybrief MetaDriver::closeImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool Si570::closeImpl()
{
    return true;
}
