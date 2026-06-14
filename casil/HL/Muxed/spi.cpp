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

#include <casil/HL/Muxed/spi.h>

#include <stdexcept>
#include <thread>
#include <utility>

using casil::Layers::HL::SPI;

CASIL_REGISTER_DRIVER_CPP(SPI)
CASIL_REGISTER_DRIVER_ALIAS("spi")

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
SPI::SPI(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig(),
       {{"RESET",     {.type{DT::Value},     .mode{AM::WriteOnly}, .addr{0},  .size{8},  .offs{0}}},
        {"VERSION",   {.type{DT::Value},     .mode{AM::ReadOnly},  .addr{0},  .size{8},  .offs{0}}},
        {"START",     {.type{DT::Value},     .mode{AM::WriteOnly}, .addr{1},  .size{8},  .offs{0}}},
        {"READY",     {.type{DT::Value},     .mode{AM::ReadOnly},  .addr{1},  .size{1},  .offs{0}}},
        {"SIZE",      {.type{DT::Value},     .mode{AM::ReadWrite}, .addr{3},  .size{16}, .offs{0}, .order{BO::Little}}},
        {"WAIT",      {.type{DT::Value},     .mode{AM::ReadWrite}, .addr{5},  .size{32}, .offs{0}, .order{BO::Little}}},
        {"REPEAT",    {.type{DT::Value},     .mode{AM::ReadWrite}, .addr{9},  .size{32}, .offs{0}, .order{BO::Little}}},
        {"EN",        {.type{DT::Value},     .mode{AM::ReadWrite}, .addr{13}, .size{1},  .offs{0}}},
        {"MEM_BYTES", {.type{DT::Value},     .mode{AM::ReadOnly},  .addr{14}, .size{16}, .offs{0}, .order{BO::Little}}}}),
    seqMemSize(0)
{
}

//Public

/*!
 * \brief Get the content of the transaction memory.
 *
 * With this function one can read \e both transaction memories (transmit sequence and readback sequence), which are adjacent
 * and have both the same size of \c MEM_BYTES bytes. If you want to read the transmit sequence for the pending transaction, set
 * \p pAddrOffs to zero, and if you want to read the readback sequence of a completed transaction, set \p pAddrOffs to \c MEM_BYTES.
 *
 * If \p pSize is -1, then \e up \e to \c MEM_BYTES bytes will be read. This depends on \p pAddrOffs, which truncates the sequence
 * from the front. If \p pAddrOffs is smaller than \c MEM_BYTES, the subsequent/remaining \e transmit sequence memory is read.
 * If \p pAddrOffs equals \c MEM_BYTES or is larger than that, the subsequent/remaining \e readback sequence memory is read.
 *
 * Note that with (in addition to \p pSize default) \p pAddrOffs defaulting to zero, the default behavior of this function
 * is to return the whole \e transmit \e sequence memory and \e not the \e readback (unlike I2C::getData(), for instance).
 *
 * Note that reading across the two different memory sections is not checked for,
 * as in principle it works fine even though it might not always make sense to do this.
 *
 * \throws std::invalid_argument If \p pSize is invalid (i.e. smaller than -1).
 * \throws std::invalid_argument If \p pSize \c + \p pAddrOffs exceeds the memory size of the %SPI module instance.
 * \throws std::invalid_argument If \p pAddrOffs exceeds the memory size of the %SPI module instance, in case of automatic \p pSize (i.e. -1).
 * \throws std::runtime_error If reading the data fails.
 *
 * \param pSize Number of bytes to read, or -1 to read \c MEM_BYTES bytes (modulo \p pAddrOffs, see above).
 * \param pAddrOffs Data offset (and memory section offset/"pointer") as number of bytes.
 * \return Current state of the %SPI transaction memory as byte sequence.
 */
std::vector<std::uint8_t> SPI::getData(const int pSize, const std::uint32_t pAddrOffs)
{
    if (pSize < -1)
        throw std::invalid_argument("Invalid number of bytes to read for SPI driver \"" + name + "\".");
    else if (pSize == -1)
    {
        if (pAddrOffs >= 2*seqMemSize)
            throw std::invalid_argument("Requested byte offset exceeds module's memory size for SPI driver \"" + name + "\".");
        else if (pAddrOffs >= seqMemSize)
            return read(seqMemOffs + pAddrOffs, 2*seqMemSize - pAddrOffs);
        else
            return read(seqMemOffs + pAddrOffs, seqMemSize - pAddrOffs);
    }
    else
    {
        if (static_cast<std::uint32_t>(pSize) + pAddrOffs > 2*seqMemSize)
            throw std::invalid_argument("Requested number of bytes to read exceeds module's memory size for SPI driver \"" + name + "\".");

        return read(seqMemOffs + pAddrOffs, pSize);
    }
}

/*!
 * \brief Set the content of the transaction memory.
 *
 * Use this to set the message/command that should be send in the next transaction.
 *
 * The available memory size can be read from the \c MEM_BYTES register.
 *
 * \throws std::invalid_argument If the length of \p pData, offset by \p pAddrOffs, exceeds the memory size of the %SPI module instance.
 * \throws std::runtime_error If writing the sequence fails.
 *
 * \param pData New data/message for the %SPI transaction memory as byte sequence.
 * \param pAddrOffs Data offset as number of bytes.
 */
void SPI::setData(const std::vector<std::uint8_t>& pData, const std::uint32_t pAddrOffs)
{
    if (pData.size() + pAddrOffs > seqMemSize)
        throw std::invalid_argument("Data length exceeds module's memory size for SPI driver \"" + name + "\".");

    write(seqMemOffs + pAddrOffs, pData);
}

/*!
 * \brief Start the transaction.
 *
 * Starts a new %SPI transaction by writing to the \c START register, which will use the payload saved in the
 * transaction memory (see setData()), possibly truncated by the transaction length set via the \c SIZE register.
 *
 * Note: The firmware module (potentially) repeats the transaction/sequence according to the state of the
 * \c REPEAT register with an inter-repetition delay according to the state of the \c WAIT register.
 *
 * \throws std::runtime_error If writing the register fails.
 */
void SPI::exec()
{
    setValue("START", 0);
}

/*!
 * \brief Check if the transaction was completed.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return If the previous transaction was completed.
 */
bool SPI::isDone()
{
    return (getValue("READY") == 0b1u);
}

//

/*!
 * \brief Start the transaction.
 *
 * This is an alias for exec().
 *
 * \copydetails exec()
 */
void SPI::start()
{
    exec();
}

/*!
 * \brief Check if the transaction was completed.
 *
 * This is an alias for isDone().
 *
 * \copydetails isDone()
 */
bool SPI::isReady()
{
    return isDone();
}

/*!
 * \brief Get the \c MEM_BYTES register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Size of the transaction memory in bytes.
 */
std::uint16_t SPI::memSize() const
{
    return static_cast<std::uint16_t>(getValue("MEM_BYTES"));
}

/*!
 * \brief Get the \c SIZE register.
 *
 * \attention This is the transaction length in \e bits, not bytes.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Configured length of the next transaction in \e bits.
 */
std::uint16_t SPI::getSize() const
{
    return static_cast<std::uint16_t>(getValue("SIZE"));
}

/*!
 * \brief Set the \c SIZE register.
 *
 * \attention This is the transaction length in \e bits, not bytes.
 *
 * If \p pSize is -1, then the value of \c MEM_BYTES times 8 is written to the \c SIZE register instead (use whole transaction memory).
 *
 * \throws std::invalid_argument If \p pSize is smaller than -1 or larger than <tt>MEM_BYTES * 8</tt> (transaction memory size in bits).
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pSize New length for the next transaction in \e bits, or -1 to use the whole transaction memory (<tt>MEM_BYTES * 8</tt> bits).
 */
void SPI::setSize(const int pSize)
{
    if (pSize < -1)
        throw std::invalid_argument("Invalid transaction length for SPI driver \"" + name + "\".");
    else if (pSize == -1)
        setValue("SIZE", seqMemSize*8);
    else if (std::cmp_greater(pSize, seqMemSize*8))
        throw std::invalid_argument("Transaction length exceeds module's memory size for SPI driver \"" + name + "\".");
    else
        setValue("SIZE", pSize);
}

/*!
 * \brief Get the \c WAIT register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Time delay between repetitions in clock cycles.
 */
std::uint32_t SPI::getWait() const
{
    return static_cast<std::uint32_t>(getValue("WAIT"));
}

/*!
 * \brief Set the \c WAIT register.
 *
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pCycles Time delay between repetitions in clock cycles.
 */
void SPI::setWait(const std::uint32_t pCycles)
{
    setValue("WAIT", pCycles);
}

/*!
 * \brief Get the \c REPEAT register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Number of repetitions of the transaction/sequence, or zero for infinite repetitions.
 */
std::uint32_t SPI::getRepeat() const
{
    return static_cast<std::uint32_t>(getValue("REPEAT"));
}

/*!
 * \brief Set the \c REPEAT register.
 *
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pReps Number of repetitions of the transaction/sequence, or zero for infinite repetitions.
 */
void SPI::setRepeat(const std::uint32_t pReps)
{
    setValue("REPEAT", pReps);
}

/*!
 * \brief Get the \c EN register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return True if the firmware module is configured to automatically start the transaction on rising \c EXT_START signal (on FPGA).
 */
bool SPI::getEn() const
{
    return ((getValue("EN") & 0x1u) == 0b1u);
}

/*!
 * \brief Set the \c EN register.
 *
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pEnable Use true to configure the firmware module to automatically start the transaction on rising \c EXT_START signal (on FPGA).
 */
void SPI::setEn(const bool pEnable)
{
    setValue("EN", pEnable ? 1 : 0);
}

/*!
 * \brief Configure and start a transaction.
 *
 * This is a convenience function for a single transaction without any readback.
 *
 * Writes the byte sequence \p pCmd into the transaction memory (see also setData()), configures the register \c SIZE
 * to match the length of \p pCmd and the registers \c REPEAT and \c WAIT to do a single transaction with no trailing
 * wait cycles. Then starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 *
 * \throws std::invalid_argument If length of \p pCmd exceeds \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 *
 * \param pCmd %Bytes to be written.
 */
void SPI::sendCmd(const std::vector<std::uint8_t>& pCmd)
{
    setSize(pCmd.size()*8);
    setWait(0);
    setRepeat(1);
    setData(pCmd);
    exec();
    while (!isDone())
        std::this_thread::yield();
}

/*!
 * \brief Configure and start a transaction.
 *
 * This is a convenience function for a single transaction without any readback.
 *
 * Writes the single byte \p pCmd into the transaction memory (see also setData()), configures the register \c SIZE to
 * match the 8 bit length of \p pCmd and the registers \c REPEAT and \c WAIT to do a single transaction with no trailing
 * wait cycles. Then starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 *
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 *
 * \param pCmd Single byte to be written.
 */
void SPI::sendCmd(const std::uint8_t pCmd)
{
    sendCmd(std::vector<std::uint8_t>{pCmd});
}

/*!
 * \brief Configure and start a transaction.
 *
 * This is a convenience function for a single transaction without any readback.
 *
 * Writes the two byte long sequence <tt>{pCmdMsb, pCmdLsb}</tt> into the transaction memory (see also setData()),
 * configures the register \c SIZE to match the 16 bit length of that sequence and the registers \c REPEAT and
 * \c WAIT to do a single transaction with no trailing wait cycles. Then starts the transaction
 * (see also exec()) and waits for its completion (see also isDone()).
 *
 * \throws std::invalid_argument If \c MEM_BYTES (transaction memory size) is smaller than two.
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 *
 * \param pCmdMsb Most significant byte of the two byte sequence to be written.
 * \param pCmdLsb Least significant byte of the two byte sequence to be written.
 */
void SPI::sendCmd(const std::uint8_t pCmdMsb, const std::uint8_t pCmdLsb)
{
    sendCmd(std::vector<std::uint8_t>{pCmdMsb, pCmdLsb});
}

/*!
 * \brief Configure and start a transaction and get the readback.
 *
 * This is a convenience function for a single transaction with readback data.
 *
 * Writes the byte sequence \p pCmd into the transaction memory (see also setData()), configures the register
 * \c SIZE to match the length of \p pCmd and the registers \c REPEAT and \c WAIT to do a single transaction
 * with no trailing wait cycles. Then starts the transaction (see also exec()) and waits for its completion
 * (see also isDone()). \p pSize bytes out of the received bytes (as obtained via getData()) will
 * be returned, or the amount of bytes that matches the length of \p pCmd if \p pSize is -1.
 *
 * \throws std::invalid_argument If \p pSize is smaller than -1.
 * \throws std::invalid_argument If \p pSize or the length of \p pCmd exceeds \c MEM_BYTES (transaction memory size).
 * \throws std::invalid_argument If \p pSize exceeds the length of \p pCmd.
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 * \throws std::runtime_error If readidng the data fails (see also getData()).
 *
 * \param pCmd %Bytes to be written.
 * \param pSize Number of bytes to read (maximally length of \p pCmd), or -1 to match length of \p pCmd.
 * \return Read bytes.
 */
std::vector<std::uint8_t> SPI::sendCmdRead(const std::vector<std::uint8_t>& pCmd, const int pSize)
{
    if (pSize < -1)
        throw std::invalid_argument("Invalid number of bytes to read for SPI driver \"" + name + "\".");
    else if (std::cmp_greater(pSize, seqMemSize))
        throw std::invalid_argument("Number of bytes to read exceeds module's memory size for SPI driver \"" + name + "\".");

    if (pCmd.size() > seqMemSize)
        throw std::invalid_argument("Command length exceeds module's memory size for SPI driver \"" + name + "\".");

    if (std::cmp_greater(pSize, pCmd.size()))
        throw std::invalid_argument("Number of bytes to read exceeds command length for SPI driver \"" + name + "\".");

    sendCmd(pCmd);

    if (pSize == -1)
        return getData(pCmd.size(), seqMemSize);
    else
        return getData(pSize, seqMemSize);
}

/*!
 * \brief Configure and start a transaction and get the readback.
 *
 * This is a convenience function for a single transaction with readback data.
 *
 * Writes the single byte \p pCmd into the transaction memory (see also setData()), configures the register
 * \c SIZE to match the 8 bit length of \p pCmd and the registers \c REPEAT and \c WAIT to do a single
 * transaction with no trailing wait cycles. Then starts the transaction (see also exec()) and waits for
 * its completion (see also isDone()). The received byte (as obtained via getData()) will be returned.
 *
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 * \throws std::runtime_error If reading the data fails (see also getData()).
 *
 * \param pCmd Single byte to be written.
 * \return Read byte.
 */
std::vector<std::uint8_t> SPI::sendCmdRead(const std::uint8_t pCmd)
{
    return sendCmdRead(std::vector<std::uint8_t>{pCmd}, 1);
}

/*!
 * \brief Configure and start a transaction and get the readback.
 *
 * This is a convenience function for a single transaction with readback data.
 *
 * Writes the two byte long sequence <tt>{pCmdMsb, pCmdLsb}</tt> into the transaction memory (see also setData()),
 * configures the register \c SIZE to match the 16 bit length of that sequence and the registers \c REPEAT and \c WAIT
 * to do a single transaction with no trailing wait cycles. Then starts the transaction (see also exec()) and waits
 * for its completion (see also isDone()). The two received bytes (as obtained via getData()) will be returned.
 *
 * \throws std::invalid_argument If \c MEM_BYTES (transaction memory size) is smaller than two.
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If writing the data fails (see also setData()).
 * \throws std::runtime_error If reading the data fails (see also getData()).
 *
 * \param pCmdMsb Most significant byte of the two byte sequence to be written.
 * \param pCmdLsb Least significant byte of the two byte sequence to be written.
 * \return Read bytes.
 */
std::vector<std::uint8_t> SPI::sendCmdRead(const std::uint8_t pCmdMsb, const std::uint8_t pCmdLsb)
{
    return sendCmdRead(std::vector<std::uint8_t>{pCmdMsb, pCmdLsb}, 2);
}

//Private

/*!
 * \copybrief RegisterDriver::initModule()
 *
 * Reads/initializes the available memory size from the \c MEM_BYTES register (hardcoded in module).
 *
 * \return True if successful.
 */
bool SPI::initModule()
{
    try
    {
        seqMemSize = static_cast<std::uint16_t>(getValue("MEM_BYTES"));
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not read module's memory size: ") + exc.what());
        return false;
    }

    return true;
}

//

/*!
 * \copybrief RegisterDriver::resetImpl()
 *
 * \throws std::runtime_error If writing the register fails.
 *
 * Sets the \c RESET register to 0.
 */
void SPI::resetImpl()
{
    setValue("RESET", 0);
}

//

/*!
 * \copybrief RegisterDriver::getModuleSoftwareVersion()
 *
 * \copydetails RegisterDriver::getModuleSoftwareVersion()
 */
std::uint8_t SPI::getModuleSoftwareVersion() const
{
    return requireFirmwareVersion;
}

/*!
 * \copybrief RegisterDriver::getModuleFirmwareVersion()
 *
 * Reads the \c VERSION register.
 *
 * \throws std::runtime_error If reading the value fails.
 *
 * \copydetails RegisterDriver::getModuleFirmwareVersion()
 */
std::uint8_t SPI::getModuleFirmwareVersion()
{
    return static_cast<std::uint8_t>(getValue("VERSION"));
}
