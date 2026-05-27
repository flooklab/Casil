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

#include <casil/HL/Muxed/i2c.h>

#include <stdexcept>
#include <thread>
#include <utility>

using casil::Layers::HL::I2C;

CASIL_REGISTER_DRIVER_CPP(I2C)
CASIL_REGISTER_DRIVER_ALIAS("i2c")

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
I2C::I2C(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig(),
       {{"RESET",     {.type{DataType::Value},     .mode{AccessMode::WriteOnly}, .addr{0},  .size{8},  .offs{0}}},
        {"VERSION",   {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{0},  .size{8},  .offs{0}}},
        {"START",     {.type{DataType::Value},     .mode{AccessMode::WriteOnly}, .addr{1},  .size{8},  .offs{0}}},
        {"READY",     {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{1},  .size{1},  .offs{0}}},
        {"NO_ACK",    {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{1},  .size{1},  .offs{1}}},
        {"ADDR",      {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{2},  .size{8},  .offs{0}}},
        {"SIZE",      {.type{DataType::Value},     .mode{AccessMode::ReadWrite}, .addr{3},  .size{16}, .offs{0}}},
        {"MEM_BYTES", {.type{DataType::Value},     .mode{AccessMode::ReadOnly},  .addr{6},  .size{16}, .offs{0}}}}),
    seqMemSize(0)
{
}

//Public

/*!
 * \brief Get the content of the transaction memory.
 *
 * This can be either the message that was previously written to the memory,
 * or the readback that was received from the slave after sending a read command.
 *
 * The available memory size can be read from the \c MEM_BYTES register.
 *
 * If \p pSize is -1, then the whole transaction memory is read (\c MEM_BYTES bytes).
 *
 * \throws std::invalid_argument If \p pSize is invalid (i.e. smaller than -1).
 * \throws std::invalid_argument If \p pSize \c + \p pAddrOffs exceeds the memory size of the %I2C module instance.
 * \throws std::invalid_argument If \p pAddrOffs exceeds the memory size of the %I2C module instance, in case of automatic \p pSize (i.e. -1).
 * \throws std::runtime_error If reading the data fails.
 *
 * \param pSize Number of bytes to read, or -1 to read the whole \c MEM_BYTES (modulo \p pAddrOffs).
 * \param pAddrOffs Data offset as number of bytes.
 * \return Current state of the %I2C transaction memory as byte sequence.
 */
std::vector<std::uint8_t> I2C::getData(const int pSize, const std::uint32_t pAddrOffs)
{
    if (pSize < -1)
        throw std::invalid_argument("Invalid number of bytes to read for I2C driver \"" + name + "\".");
    else if (pSize == -1)
    {
        if (pAddrOffs > seqMemSize)
            throw std::invalid_argument("Requested byte offset exceeds module's memory size for I2C driver \"" + name + "\".");

        return read(seqMemOffs + pAddrOffs, seqMemSize - pAddrOffs);
    }
    else
    {
        if (static_cast<std::uint32_t>(pSize) + pAddrOffs > seqMemSize)
            throw std::invalid_argument("Requested number of bytes to read exceeds module's memory size for I2C driver \"" + name + "\".");

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
 * \throws std::invalid_argument If length of \p pData offset by \p pAddrOffs exceeds the memory size of the %I2C module instance.
 * \throws std::runtime_error If writing the sequence fails.
 *
 * \param pData New data/message for the %I2C transaction memory as byte sequence.
 * \param pAddrOffs Data offset as number of bytes.
 */
void I2C::setData(const std::vector<std::uint8_t>& pData, const std::uint32_t pAddrOffs)
{
    if (pData.size() + pAddrOffs > seqMemSize)
        throw std::invalid_argument("Data length exceeds module's memory size for I2C driver \"" + name + "\".");

    write(seqMemOffs + pAddrOffs, pData);
}

/*!
 * \brief Start the transaction.
 *
 * Starts a new %I2C transaction by writing to the \c START register, which will use the payload saved in the transaction memory
 * (see setData()), possibly truncated by the transaction length set via the \c SIZE register. Uses the \c ADDR register to
 * determine the slave address (<tt>ADDR[7:1]</tt>) and whether this is a read/write transaction (read if <tt>ADDR[0] == 1</tt>).
 *
 * \throws std::runtime_error If writing the register fails.
 */
void I2C::exec()
{
    setValue("START", 0);
}

/*!
 * \brief Check if the transaction was completed.
 *
 * \throws std::runtime_error If the transaction was \e not acknowledged (received \c NACK bit from slave).
 * \throws std::runtime_error If reading any of the registers fails.
 *
 * \return If the previous transaction was completed.
 */
bool I2C::isDone()
{
    if (getValue("NO_ACK") == 0b1u)
        throw std::runtime_error("Transaction not acknowledged for I2C driver \"" + name + "\".");

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
void I2C::start()
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
bool I2C::isReady()
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
std::uint16_t I2C::memSize() const
{
    return static_cast<std::uint16_t>(getValue("MEM_BYTES"));
}

/*!
 * \brief Get the \c ADDR register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Full value of the address register including read/write (1/0) bit in LSB position.
 */
std::uint8_t I2C::getAddr() const
{
    return static_cast<std::uint8_t>(getValue("ADDR"));
}

/*!
 * \brief Set the \c ADDR register.
 *
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pAddr Full value of the address register including read/write (1/0) bit in LSB position.
 */
void I2C::setAddr(const std::uint8_t pAddr)
{
    setValue("ADDR", pAddr);
}

/*!
 * \brief Get slave address from \c ADDR register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Configured %I2C slave address, which is <tt>ADDR[7:1]</tt>.
 */
std::bitset<7> I2C::getSlaveAddr() const
{
    return std::bitset<7>(getAddr() >> 1);
}

/*!
 * \brief Set slave address for \c ADDR register.
 *
 * \throws std::runtime_error If reading or writing the register fails.
 *
 * \param pAddr New %I2C slave address, which is written to <tt>ADDR[7:1]</tt>.
 */
void I2C::setSlaveAddr(const std::bitset<7>& pAddr)
{
    setAddr((pAddr.to_ulong() << 1) | (getAddr() & 0x01u));
}

/*!
 * \brief Get read/write bit from \c ADDR register.
 *
 * Gets the configured read/write bit (for the next transaction), which is <tt>ADDR[0]</tt>.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return True for read mode and false for write mode.
 */
bool I2C::getReadWriteBit() const
{
    return ((getAddr() & 0x01u) == 0b1u);
}

/*!
 * \brief Set read/write bit for \c ADDR register.
 *
 * Sets the read/write bit (for the next transaction), which is written to <tt>ADDR[0]</tt>.
 *
 * \throws std::runtime_error If reading or writing the register fails.
 *
 * \param pRead Set read mode if true and write mode else.
 */
void I2C::setReadWriteBit(const bool pRead)
{
    if (pRead)
        setAddr(getAddr() | 0x01u);
    else
        setAddr(getAddr() & 0xFEu);
}

/*!
 * \brief Get the \c SIZE register.
 *
 * \throws std::runtime_error If reading the register fails.
 *
 * \return Configured length of the next transaction in bytes.
 */
std::uint16_t I2C::getSize() const
{
    return static_cast<std::uint16_t>(getValue("SIZE"));
}

/*!
 * \brief Set the \c SIZE register.
 *
 * If \p pSize is -1, then the value of \c MEM_BYTES is written to the \c SIZE register instead (use whole transaction memory).
 *
 * \throws std::invalid_argument If \p pSize is smaller than -1 or larger than \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If writing the register fails.
 *
 * \param pSize New length for the next transaction in bytes, or -1 to use the whole transaction memory (\c MEM_BYTES bytes).
 */
void I2C::setSize(const int pSize)
{
    if (pSize < -1)
        throw std::invalid_argument("Invalid number of bytes to read for I2C driver \"" + name + "\".");
    else if (pSize == -1)
        setValue("SIZE", seqMemSize);
    else if (std::cmp_greater(pSize, seqMemSize))
        throw std::invalid_argument("Transaction length exceeds module's memory size for I2C driver \"" + name + "\".");
    else
        setValue("SIZE", pSize);
}

/*!
 * \brief Configure and start a read transaction.
 *
 * Configures the registers \c ADDR and \c SIZE for a read transaction to/from %I2C address \p pAddr[7:1] (\p pAddr[0] is ignored)
 * that receives \p pSize bytes. Starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 * The received bytes (as obtained via getData()) will be returned.
 *
 * \throws std::invalid_argument If \p pSize is smaller than -1 or larger than \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If the transaction was \e not acknowledged (received \c NACK bit from slave, see also isDone()).
 * \throws std::runtime_error If reading the data fails (see also getData()).
 *
 * \param pAddr Value for the \c ADDR register as required by setAddr() (except that \p pAddr[0] will be ignored).
 * \param pSize Number of bytes to read.
 * \return Read bytes.
 */
std::vector<std::uint8_t> I2C::sendRead(const std::uint8_t pAddr, const int pSize)
{
    setAddr((pAddr << 1) | 0x01u);
    setSize(pSize);
    exec();
    while (!isDone())
        std::this_thread::yield();

    return getData(pSize);
}

/*!
 * \brief Configure and start a read transaction.
 *
 * Configures the registers \c ADDR and \c SIZE for a read transaction to/from %I2C address \p pAddr that receives \p pSize bytes.
 * Starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 * The received bytes (as obtained via getData()) will be returned.
 *
 * \throws std::invalid_argument If \p pSize is smaller than -1 or larger than \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If the transaction was \e not acknowledged (received \c NACK bit from slave, see also isDone()).
 * \throws std::runtime_error If reading the data fails (see also getData()).
 *
 * \param pAddr The seven-bit %I2C address.
 * \param pSize Number of bytes to read.
 * \return Read bytes.
 */
std::vector<std::uint8_t> I2C::sendRead(const std::bitset<7>& pAddr, const int pSize)
{
    return sendRead(pAddr.to_ulong(), pSize);
}

/*!
 * \brief Configure and start a write transaction.
 *
 * Configures the registers \c ADDR and \c SIZE for a write transaction to %I2C address \p pAddr[7:1] (\p pAddr[0] is ignored) that
 * sends the byte sequence \p pData. Starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 *
 * \throws std::invalid_argument If length of \p pData exceeds \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If the transaction was \e not acknowledged (received \c NACK bit from slave, see also isDone()).
 * \throws std::runtime_error If writing the data fails (see also setData()).
 *
 * \param pAddr Value for the \c ADDR register as required by setAddr() (except that \p pAddr[0] will be ignored).
 * \param pData %Bytes to be written.
 */
void I2C::sendWrite(const std::uint8_t pAddr, const std::vector<std::uint8_t>& pData)
{
    setAddr((pAddr << 1) & 0xFEu);
    setSize(pData.size());
    setData(pData);
    exec();
    while (!isDone())
        std::this_thread::yield();
}

/*!
 * \brief Configure and start a write transaction.
 *
 * Configures the registers \c ADDR and \c SIZE for a write transaction to %I2C address \p pAddr that sends the
 * byte sequence \p pData. Starts the transaction (see also exec()) and waits for its completion (see also isDone()).
 *
 * \throws std::invalid_argument If length of \p pData exceeds \c MEM_BYTES (transaction memory size).
 * \throws std::runtime_error If reading or writing any of the registers fails.
 * \throws std::runtime_error If the transaction was \e not acknowledged (received \c NACK bit from slave, see also isDone()).
 * \throws std::runtime_error If writing the data fails (see also setData()).
 *
 * \param pAddr The seven-bit %I2C address.
 * \param pData %Bytes to be written.
 */
void I2C::sendWrite(const std::bitset<7>& pAddr, const std::vector<std::uint8_t>& pData)
{
    sendWrite(pAddr.to_ulong(), pData);
}

//Private

/*!
 * \copybrief RegisterDriver::initModule()
 *
 * Reads/initializes the available memory size from the \c MEM_BYTES register (hardcoded in module).
 *
 * \return True if successful.
 */
bool I2C::initModule()
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
void I2C::resetImpl()
{
    setValue("RESET", 0);
}

//

/*!
 * \copybrief RegisterDriver::getModuleSoftwareVersion()
 *
 * \copydetails RegisterDriver::getModuleSoftwareVersion()
 */
std::uint8_t I2C::getModuleSoftwareVersion() const
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
std::uint8_t I2C::getModuleFirmwareVersion()
{
    return static_cast<std::uint8_t>(getValue("VERSION"));
}
