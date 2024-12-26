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

#include <casil/TL/Muxed/sitcp.h>

#include <casil/bytes.h>
#include <casil/logger.h>

#include <algorithm>
#include <bitset>
#include <chrono>
#include <iterator>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>

using casil::Layers::TL::SiTCP;

CASIL_REGISTER_INTERFACE_CPP(SiTCP)
CASIL_REGISTER_INTERFACE_ALIAS("SiTcp")

//

/*!
 * \brief Constructor.
 *
 * Initializes the host name to connect to from the mandatory "init.ip" string in \p pConfig.
 *
 * Initializes the network port for the %UDP communication from the mandatory "init.udp_port" value (integer type) in \p pConfig.
 *
 * Initializes the network port for the optional %TCP connection (see below) from the optional "init.tcp_port" value
 * in \p pConfig (integer type, default: 0).
 *
 * Enables using the %TCP connection depending on the optional "init.tcp_connection" value in \p pConfig (boolean type, default: false).
 *
 * Enables using the %TCP connection (instead of %UDP) for normal bus writes ("tcp_to_bus" feature, see write()) depending
 * on the optional "init.tcp_to_bus" value (boolean type, default: false) in \p pConfig (requires enabled "init.tcp_connection").
 *
 * Initializes the timeout for establishing a %TCP/%UDP connection (see init()) from the optional "init.connect_timeout" value
 * in \p pConfig (floating-point value in seconds, default: 5.0).
 *
 * \throws std::runtime_error If "init.ip" is empty.
 * \throws std::runtime_error If "init.udp_port" is out of range (must be in <tt>(0, 65535]</tt>).
 * \throws std::runtime_error If %TCP connection is enabled and "init.tcp_port" is out of range (must be in <tt>(0, 65535]</tt>).
 * \throws std::runtime_error If "init.tcp_to_bus" is enabled but %TCP connection is disabled.
 * \throws std::runtime_error For negative connect timeouts.
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
SiTCP::SiTCP(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig::fromYAML(
                       "{init: {ip: string, udp_port: int}}")
                    ),
    hostName(config.getStr("init.ip", "")),
    udpPort(config.getInt("init.udp_port", 0)),
    tcpPort(config.getInt("init.tcp_port", 0)),
    useTcp(config.getBool("init.tcp_connection", false)),
    useTcpToBus(config.getBool("init.tcp_to_bus", false)),
    connectTimeoutSecs(config.getDbl("init.connect_timeout", 5.0)),
    connectTimeout(Auxil::getChronoMilliSecs(connectTimeoutSecs)),
    udpSocketWrapper(hostName, udpPort),
    tcpSocketWrapperPtr(useTcp ? std::make_unique<CommonImpl::TCPSocketWrapper>(hostName, tcpPort, "", "") : nullptr),
    fifoThread(),
    fifoBuffer(),
    fifoMutex(),
    tcpSocketMutex(),
    wantLockTCPSocket(ATOMIC_FLAG_INIT),
    pollFIFO(false),
    rbcpId(0)
{
    if (hostName == "")
        throw std::runtime_error("No address/hostname set for " + getSelfDescription() + ".");
    if (udpPort <= 0 || udpPort > 65535)
        throw std::runtime_error("Invalid UDP port number set for " + getSelfDescription() + ".");
    if (useTcp && (tcpPort <= 0 || tcpPort > 65535))
        throw std::runtime_error("Invalid TCP port number set for " + getSelfDescription() + ".");
    if (useTcpToBus && !useTcp)
        throw std::runtime_error("Contradictory TCP settings for " + getSelfDescription() + ".");
    if (connectTimeoutSecs < 0.0)
        throw std::runtime_error("Negative connect timeout set for " + getSelfDescription() + ".");
}

/*!
 * \brief Destructor.
 *
 * Calls close() if still initialized (i.e. init() called but close() not called yet or failed).
 */
SiTCP::~SiTCP()
{
    if (initialized)    //Not closed yet; need to stop FIFO thread
        close(true);
}

//Public

/*!
 * \copybrief MuxedInterface::read()
 *
 * What this function does depends on the value of \p pAddr:
 * - <tt>[0, \ref baseAddrDataLimit)</tt>: Reads \p pSize bytes from the basil bus at \p pAddr (uses RBCP messages over %UDP).
 * - <tt>[\ref baseAddrDataLimit, \ref baseAddrFIFOLimit)</tt>: Returns SiTcp FIFO data, see getFifoData() with \p pSize as argument.
 * - <tt>baseAddrFIFOLimit</tt>: ... not implemented yet
 * - <tt>[baseAddrFIFOLimit, ...)</tt>: Returns SiTcp FIFO size (see getFifoSize()) as 4 byte long little endian sequence
 *                                      if \p pSize is 4 and \p pSize zeros otherwise.
 *
 * \todo update baseAddrFIFOLimit when implemented
 *
 * \throws std::runtime_error For negative \p pSize, if also \p pAddr < \ref baseAddrDataLimit.
 * \throws std::runtime_error If the RBCP read fails (invalid/wrong/non-matching RBCP response, timeout, failed %UDP socket access).
 *
 * \copydetails MuxedInterface::read()
 */
std::vector<std::uint8_t> SiTCP::read(const std::uint64_t pAddr, const int pSize)
{
    if (pAddr < baseAddrDataLimit)
    {
        if (pSize < 0)
            throw std::runtime_error("Could not read from SiTcp socket \"" + name + "\": Requested read size is invalid in this context.");

        try
        {
            if (pSize <= rbcpMaxSize)
                return readSingle(pAddr, pSize);
            else
            {
                std::vector<std::uint8_t> retVal;

                std::uint32_t currentAddr = pAddr;

                for (int i = 0; i < pSize/rbcpMaxSize; ++i)
                {
                    Bytes::appendToByteVec(retVal, readSingle(currentAddr, rbcpMaxSize));

                    currentAddr += rbcpMaxSize;
                }
                if (pSize % rbcpMaxSize > 0)
                    Bytes::appendToByteVec(retVal, readSingle(currentAddr, pSize % rbcpMaxSize));

                return retVal;
            }
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error("Could not read from SiTcp socket \"" + name + "\". Call to readSingle() failed: " + exc.what());
        }
    }
    else if (pAddr < baseAddrFIFOLimit)
    {
        return getFifoData(pSize);
    }
    else if (pAddr == baseAddrFIFOLimit)
    {
        return {};
        //TODO What is actually going on here?:
        //Python: return array('B', chr(sram_fifo_version))
        //This probably fakes reading VERSION register of sram_fifo module at base_addr 0x200000000, not sure about purpose of chr() though...

        //TODO more python context:
        //from basil.HL.sram_fifo import sram_fifo
        //sram_fifo_version = int(re.findall(r'\d+', sram_fifo._require_version)[-1])
        //# Fake SRAM version to ensure compatibility with the simulation
    }
    else    //TODO Basil comment: "this is to fake a HL fifo. Is there better way? Definitely..."
    {
        if (pSize == 4)
            return Bytes::composeByteVec(false, static_cast<std::uint32_t>(getFifoSize()));
        else
        {
            return std::vector<std::uint8_t>(pSize, '\0');  //TODO Basil comment: "FIXME: workaround for SRAM module registers"
            //TODO this was commented out in basil (???): #logger.warning("SiTcp:read - Invalid address %s" % hex(addr))
        }
    }
}

/*!
 * \copybrief MuxedInterface::write()
 *
 * What this function does depends on the value of \p pAddr:
 * - <tt>[0, \ref baseAddrDataLimit)</tt>: Writes \p pData to the basil bus at \p pAddr (uses RBCP messages over %UDP,
 *                                         or a "tcp_to_bus" message over %TCP if "tcp_to_bus" is enabled).
 * - <tt>[\ref baseAddrDataLimit, \ref baseAddrFIFOLimit)</tt>: Writes raw \p pData to the %TCP socket (e.g. writing
 *                                                              \p pData to the SiTcp FIFO if "tcp_to_bus" is \e not enabled).
 * - <tt>baseAddrFIFOLimit</tt>: Calls resetFifo().
 *
 * \throws std::runtime_error If "tcp_to_bus" enabled and \p pData exceeds the maximum data length for %TCP bus writes (\c 0xFFF9u ),
 *                            if also \p pAddr < \ref baseAddrDataLimit.
 * \throws std::runtime_error If \p pAddr exceeds \ref baseAddrFIFOLimit.
 * \throws std::runtime_error If writing to or clearing the FIFO but %TCP connection not enabled.
 * \throws std::runtime_error If the RBCP write fails (invalid/wrong/non-matching RBCP response, timeout, failed %UDP socket access)
 *                            or if writing to the %TCP socket fails.
 * \throws std::runtime_error If resetting the FIFO fails.
 *
 * \copydetails MuxedInterface::write()
 */
void SiTCP::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    if (pAddr < baseAddrDataLimit)
    {
        if (useTcp && useTcpToBus)
        {
            if (pData.size() > 0xFFF9u)
                throw std::runtime_error("Could not write to SiTcp socket \"" + name + "\": Data length exceeds maximum RBCP data length.");

            std::vector<std::uint8_t> sendData;

            sendData.reserve(pData.size() + 6);

            Bytes::appendToByteVec(sendData,
                                   Bytes::composeByteVec(false, static_cast<std::uint16_t>(pData.size()), static_cast<std::uint32_t>(pAddr)));

            std::copy(pData.begin(), pData.end(), std::back_inserter(sendData));

            try
            {
                if (tcpSocketWrapperPtr)
                    tcpSocketWrapperPtr->write(sendData);
                else
                    throw std::runtime_error("Undefined TCP socket. THIS SHOULD NEVER HAPPEN!");
            }
            catch (const std::runtime_error& exc)
            {
                throw std::runtime_error("Could not write to SiTcp socket \"" + name + "\": " + exc.what());
            }
        }
        else
        {
            std::uint32_t currentAddr = pAddr;
            auto nFullWrites = pData.size()/rbcpMaxSize;

            auto currentDataIt = pData.begin();

            try
            {
                for (auto i = decltype(nFullWrites){0}; i < nFullWrites; ++i)
                {
                    writeSingle(currentAddr, std::vector<std::uint8_t>(currentDataIt, currentDataIt+rbcpMaxSize));

                    currentAddr += rbcpMaxSize;
                    currentDataIt += rbcpMaxSize;
                }

                if (pData.size() % rbcpMaxSize > 0)
                    writeSingle(currentAddr, std::vector<std::uint8_t>(currentDataIt, pData.end()));
            }
            catch (const std::runtime_error& exc)
            {
                throw std::runtime_error("Could not write to SiTcp socket \"" + name + "\". Call to writeSingle() failed: " + exc.what());
            }
        }
    }
    else if (pAddr < baseAddrFIFOLimit)
    {
        try
        {
            if (tcpSocketWrapperPtr)
                tcpSocketWrapperPtr->write(pData);  //TODO Basil comment: chunking?
            else
                throw std::runtime_error("Undefined TCP socket.");
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error("Could not write to SiTcp socket \"" + name + "\": " + exc.what());
        }
    }
    else if (pAddr == baseAddrFIFOLimit)
    {
        resetFifo();
    }
    else
    {
        throw std::invalid_argument("Invalid address " + Bytes::formatHex(pAddr) + " for writing to " + getSelfDescription());
    }
}

/*!
 * \copybrief MuxedInterface::query()
 *
 * Does nothing.
 *
 * \param pWriteAddr Ignored.
 * \param pReadAddr Ignored.
 * \param pData Ignored.
 * \param pSize Ignored.
 * \return Empty vector.
 */
std::vector<std::uint8_t> SiTCP::query(std::uint64_t, std::uint64_t, const std::vector<std::uint8_t>&, int)
{
    Logger::logWarning("The query() function is not implemented for the SiTcp interface (does nothing).");
    return {};
}

//

/*!
 * \brief Check if the %UDP read buffer is empty.
 *
 * \throws std::runtime_error If checking the buffer size fails.
 *
 * \copydetails MuxedInterface::readBufferEmpty()
 */
bool SiTCP::readBufferEmpty() const
{
    try
    {
        return udpSocketWrapper.readBufferEmpty();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not check UDP read buffer size of SiTcp socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \brief Clear the current contents of the %UDP read buffer.
 *
 * \throws std::runtime_error If clearing the buffer fails.
 */
void SiTCP::clearReadBuffer()
{
    try
    {
        udpSocketWrapper.clearReadBuffer();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear UDP read buffer of SiTcp socket \"" + name + "\": " + exc.what());
    }
}

//

/*!
 * \brief Clear the FIFO and the remaining incoming %TCP buffer.
 *
 * Clears the read buffer of the %TCP socket and clears the already read/polled bytes from the FIFO.
 *
 * \throws std::runtime_error If clearing the socket buffer fails.
 */
void SiTCP::resetFifo()
{
    {
        //Make it easier/quicker to acquire the lock using this flag to notify FIFO thread
        Auxil::AtomicFlagGuard flagGuard(wantLockTCPSocket);
        (void)flagGuard;

        const std::lock_guard<std::mutex> socketLock(tcpSocketMutex);
        (void)socketLock;

        try
        {
            if (tcpSocketWrapperPtr)
                tcpSocketWrapperPtr->clearReadBuffer();
            else
                throw std::runtime_error("Undefined TCP socket.");
        }
        catch (const std::runtime_error& exc)
        {
            throw std::runtime_error("Could not properly clear FIFO of SiTcp socket \"" + name + "\": " + exc.what());
        }
    }
    {
        const std::lock_guard<std::mutex> bufferLock(fifoMutex);
        (void)bufferLock;

        fifoBuffer.clear();
    }
}

/*!
 * \brief Get the FIFO size in number of bytes.
 *
 * \return SiTcp FIFO size in bytes.
 */
std::size_t SiTCP::getFifoSize() const
{
    const std::lock_guard<std::mutex> bufferLock(fifoMutex);
    (void)bufferLock;

    return fifoBuffer.size();
}

/*!
 * \brief Extract the current FIFO content as sequence of bytes.
 *
 * The requested size \p pSize gets limited by the current FIFO size and then reduced by modulo 4 in order to
 * obtain only multiples of 4 bytes. Those multiples of 4 bytes are removed from the SiTcp FIFO and then returned.
 *
 * \param pSize Number of FIFO bytes to get (automatically reduced by modulo 4).
 * \return Byte sequence from the SiTcp FIFO in multiples of 4 bytes, according to the smaller of \p pSize or FIFO size.
 */
std::vector<std::uint8_t> SiTCP::getFifoData(const int pSize)
{
    if (pSize == 0)
        return {};

    const std::lock_guard<std::mutex> bufferLock(fifoMutex);
    (void)bufferLock;

    std::size_t byteCount;

    if (pSize < 0 || std::cmp_less(fifoBuffer.size(), pSize))
        byteCount = fifoBuffer.size();
    else
        byteCount = pSize;

    byteCount = byteCount - byteCount % 4;

    std::vector<std::uint8_t> retVal(fifoBuffer.begin(), fifoBuffer.begin() + byteCount);

    fifoBuffer.erase(fifoBuffer.begin(), fifoBuffer.begin() + byteCount);

    return retVal;
}

//Private

/*!
 * \copybrief MuxedInterface::initImpl()
 *
 * Connects the %UDP socket and, if %TCP is enabled (see SiTCP()), the %TCP socket to the configured host name
 * using the respective configured ports.
 *
 * If %TCP is enabled, resets the FIFO and starts a polling thread for the incoming FIFO data (see also pollFifo()).
 * Furthermore, if also "tcp_to_bus" is enabled, configures the SiTcp core accordingly by calling enableTcpToBus().
 *
 * Note: There must be no FIFO thread already running before starting a new one (see also close() / closeImpl()).
 *
 * \return True if successful.
 */
bool SiTCP::initImpl()
{
    try
    {
        udpSocketWrapper.init(connectTimeout);

        if (useTcp)
        {
            if (tcpSocketWrapperPtr)
                tcpSocketWrapperPtr->init(connectTimeout);
            else
                throw std::runtime_error("Undefined TCP socket. THIS SHOULD NEVER HAPPEN!");
        }
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not connect socket of " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    if (useTcp)
    {
        try
        {
            if (fifoThread.joinable())
                throw std::runtime_error("FIFO thread is already running.");

            resetFifo();

            pollFIFO.store(true);

            fifoThread = std::thread(&SiTCP::pollFifo, this);
        }
        catch (const std::system_error& exc)
        {
            Logger::logError("Could not start FIFO thread for " + getSelfDescription() + ": " + exc.what());
            return false;
        }
        catch (const std::runtime_error& exc)
        {
            Logger::logError("Could not start FIFO thread for " + getSelfDescription() + ": " + exc.what());
            return false;
        }

        if (useTcpToBus)
        {
            try
            {
                enableTcpToBus();
            }
            catch (const std::runtime_error& exc)
            {
                Logger::logError("Could not enable \"tcp_to_bus\" feature for " + getSelfDescription() + ": " + exc.what());
                return false;
            }
        }
    }

    return true;
}

/*!
 * \copybrief MuxedInterface::closeImpl()
 *
 * If %TCP is enabled, stops the polling thread started by initImpl() and disconnects the %TCP socket.
 *
 * Disconnects the %UDP socket.
 *
 * \return True if successful.
 */
bool SiTCP::closeImpl()
{
    try
    {
        if (useTcp)
        {
            pollFIFO.store(false);

            try
            {
                if (fifoThread.joinable())
                    fifoThread.join();
            }
            catch (const std::system_error& exc)
            {
                Logger::logWarning("Could not join FIFO thread of " + getSelfDescription() + ": " + exc.what());
            }
        }

        udpSocketWrapper.close();

        if (tcpSocketWrapperPtr)
            tcpSocketWrapperPtr->close();
    }
    catch (const std::runtime_error& exc)
    {
        Logger::logError("Could not close socket connection of " + getSelfDescription() + ": " + exc.what());
        return false;
    }

    return true;
}

//

/*!
 * \brief Enable using %TCP protocol for normal bus writes.
 *
 * Configures the SiTcp core for receiving normal basil bus writes via %TCP (the "tcp_to_bus" feature)
 * by writing the reset sequence <tt>{65535 * 0xFF + 6 * 0x00}</tt> (see SiTCP) to the %TCP socket.
 *
 * Returns immediately if "tcp_to_bus" is not enabled.
 *
 * \throws std::runtime_error If writing to the %TCP socket fails.
 */
void SiTCP::enableTcpToBus()
{
    if (!tcpSocketWrapperPtr)
    {
        if (useTcpToBus)
            throw std::runtime_error("Undefined TCP socket. THIS SHOULD NEVER HAPPEN!");
        else
            Logger::logWarning("Cannot use \"tcp_to_bus\" feature for " + getSelfDescription() + ": \"tcp_to_bus\" is not enabled.");

        return;
    }

    try
    {
        tcpSocketWrapperPtr->write(std::vector<std::uint8_t>(65535, 255));
        tcpSocketWrapperPtr->write(std::vector<std::uint8_t>(6, 0));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error(std::string("Could not write sequence to enable \"tcp_to_bus\": ") + exc.what());
    }
}

//

/*!
 * \brief Continuously poll the %TCP socket for new FIFO data (use as thread).
 *
 * Starts a loop to iteratively poll the SiTcp FIFO and to add new data to the FIFO buffer, if the %TCP connection
 * is enabled (see SiTCP()) and FIFO polling was enabled (see init()). Otherwise returns immediately.
 *
 * The polling stops and the function returns when polling gets disabled explicitly (see close()) or automatically as soon
 * as the maximum error count (see \ref maxFIFOErrorCount) for reads from the %TCP socket is exceeded.
 *
 * Ensures a minimum time of \ref tcpReadoutInterval between subsequent read attempts.
 */
void SiTCP::pollFifo()
{
    if (!tcpSocketWrapperPtr)
    {
        Logger::logError("Cannot poll FIFO of " + getSelfDescription() + ": Undefined TCP socket.");
        return;
    }

    std::vector<std::uint8_t> tmpBuffer;

    std::size_t errorCount = 0;

    auto lastTime = std::chrono::steady_clock::now();

    while (pollFIFO.load())
    {
        if (wantLockTCPSocket.test())
            std::this_thread::yield();  //If flag temporarily set (e.g. by resetFifo()), significantly reduces time to acquire tcpSocketMutex

        {
            const std::lock_guard<std::mutex> socketLock(tcpSocketMutex);
            (void)socketLock;

            try
            {
                tmpBuffer = tcpSocketWrapperPtr->readMax(1024*8, tcpReadoutInterval);
            }
            catch (const std::runtime_error& exc)
            {
                Logger::logError("Error while polling FIFO of " + getSelfDescription() + ": " + exc.what());

                if (++errorCount > maxFIFOErrorCount)
                {
                    pollFIFO.store(false);
                    Logger::logCritical("Exceeded maximum error count while polling FIFO of " + getSelfDescription() + ". Stopping...");
                }
            }
        }

        if (tmpBuffer.size() > 0)
        {
            const std::lock_guard<std::mutex> bufferLock(fifoMutex);
            (void)bufferLock;

            fifoBuffer.insert(fifoBuffer.end(), tmpBuffer.begin(), tmpBuffer.end());

            tmpBuffer.clear();
        }

        auto currentTime = std::chrono::steady_clock::now();
        auto evolvedTime = currentTime - lastTime;

        if (evolvedTime < tcpReadoutInterval)
        {
            std::this_thread::sleep_for(tcpReadoutInterval - evolvedTime);
            lastTime = std::chrono::steady_clock::now();
        }
        else
            lastTime = currentTime;
    }
}

//

/*!
 * \brief Read from the bus with a single RBCP request/response.
 *
 * Writes a single RBCP read message (initiating a bus read) for address \p pAddr to the basil bus, requesting to read \p pSize bytes.
 * Waits for the response message, extracts the read byte sequence from it and returns the sequence.
 *
 * See doSingleRBCPOperation() in "read mode" for further details.
 *
 * \throws std::runtime_error If \p pSize exceeds the maximum RBCP data length.
 * \throws std::runtime_error If an invalid/wrong/non-matching RBCP response message was received.
 * \throws std::runtime_error If reading/writing from/to the %UDP socket time out too often (exceeded retry limit).
 * \throws std::runtime_error If reading/writing from/to the %UDP socket fails due to non-timeout reasons.
 *
 * \param pAddr Bus address as source location for the return value.
 * \param pSize Length of the data to be read from bus address \p pAddr.
 * \return Data read from the bus at address \p pAddr.
 */
std::vector<std::uint8_t> SiTCP::readSingle(const std::uint32_t pAddr, const std::uint8_t pSize)
{
    return doSingleRBCPOperation(pAddr, pSize).value();
}

/*!
 * \brief Write to the bus with a single RBCP request/response.
 *
 * Writes a single RBCP write message (initiating a bus write) for address \p pAddr to the basil bus, requesting to write \p pData.
 * Waits for the response message in order to check that the sent message was correctly transmitted.
 *
 * See doSingleRBCPOperation() in "write mode" for further details.
 *
 * \throws std::runtime_error If length of \p pData exceeds the maximum RBCP data length.
 * \throws std::runtime_error If an invalid/wrong/non-matching RBCP response message was received.
 * \throws std::runtime_error If reading/writing from/to the %UDP socket time out too often (exceeded retry limit).
 * \throws std::runtime_error If reading/writing from/to the %UDP socket fails due to non-timeout reasons.
 *
 * \param pAddr Bus address as destination for writing \p pData.
 * \param pData Data to be written to bus address \p pAddr.
 */
void SiTCP::writeSingle(const std::uint32_t pAddr, const std::vector<std::uint8_t>& pData)
{
    doSingleRBCPOperation(pAddr, pData);
}

//

/*!
 * \brief Send a single RBCP read or write request to the bus and process the response message.
 *
 * Writes a single RBCP read \e or write message (depending on the type in \p pSizeOrData) to the SiTcp core.
 * This message initiates a read/write from/to the basil bus at bus address \p pAddr, respectively.
 * In case of "read mode", \p pSizeOrData specifies the number of bytes to read.
 * In case of "write mode", \p pSizeOrData specifies the byte sequence to be written.
 *
 * Waits for the response message in order to check that the sent message was correctly transmitted and, in case of "read mode",
 * to receive the data that was requested to be read. The read data will be returned, or nothing in case of "write mode".
 *
 * See SiTCP for detailed protocol information.
 *
 * Note: Uses timeout \ref udpTimeout for every socket read and write
 * and retries every timed out read and write \ref udpRetransmitCnt times.
 *
 * \throws std::runtime_error If either the size in or the length of the data in \p pSizeOrData exceed the maximum RBCP data length.
 * \throws std::runtime_error If an invalid/wrong/non-matching RBCP response message was received.
 * \throws std::runtime_error If reading from the %UDP socket times out more than \ref udpRetransmitCnt times.
 * \throws std::runtime_error If writing to the %UDP socket times out more than \ref udpRetransmitCnt times.
 * \throws std::runtime_error If reading/writing from/to the %UDP socket fails due to non-timeout reasons.
 *
 * \param pAddr Bus address as source/target location for reading/writing \p pSizeOrData.
 * \param pSizeOrData Either length of the data to be read from ("read mode") or data to be written to ("write mode") bus address \p pAddr.
 * \return Nothing for "write mode" and data read from the bus at address \p pAddr for "read mode".
 */
std::optional<std::vector<std::uint8_t>> SiTCP::doSingleRBCPOperation(const std::uint32_t pAddr, const std::variant<
                                                                 std::uint8_t,
                                                                 std::reference_wrapper<const std::vector<std::uint8_t>>> pSizeOrData)
{
    auto extractSize = [pSizeOrData]() -> std::uint8_t
    {
        return std::get<std::uint8_t>(pSizeOrData);
    };
    auto extractData = [pSizeOrData]() -> const std::vector<std::uint8_t>&
    {
        return std::get<std::reference_wrapper<const std::vector<std::uint8_t>>>(pSizeOrData);
    };

    enum class RBCPOperation { Read, Write };
    const RBCPOperation operationType = (std::holds_alternative<std::uint8_t>(pSizeOrData) ? RBCPOperation::Read : RBCPOperation::Write);

    if (operationType == RBCPOperation::Read && extractSize() > rbcpMaxSize)
        throw std::runtime_error("Requested read data length exceeds maximum RBCP data length.");
    else if (operationType == RBCPOperation::Write && extractData().size() > rbcpMaxSize)
        throw std::runtime_error("Length of passed data exceeds maximum RBCP data length.");

    const std::string functionName = ((operationType == RBCPOperation::Read) ? "readSingle()" : "writeSingle()");

    /*
     * Define lambda to make sure there are no unwanted datagrams left on the UDP socket; otherwise remove and
     * log a warning message for each of those (try to extract RBCP message ID and add it to the message);
     * 'pWarnMsgContext' adds context to the message (i.e. when this check is executed (see usage below))
     */
    auto checkAndClearReadBuffer = [this, functionName](const std::string& pWarnMsgContext)
    {
        while (!udpSocketWrapper.readBufferEmpty())
        {
            std::vector<std::uint8_t> tmpData = udpSocketWrapper.readMax(3);    //Read just enough for the header

            if (tmpData.size() == 3)
            {
                Logger::logWarning("Found unexpected datagram on " + getSelfDescription() + " " + pWarnMsgContext +
                                   " (in " + functionName + "). RBCP message ID: " + std::to_string(rbcpId) + " (expected), " +
                                                                                     std::to_string(tmpData[2]) + " (received).");
            }
            else
            {
                Logger::logWarning("Found unexpected datagram on " + getSelfDescription() + " " + pWarnMsgContext +
                                   " (in " + functionName + ").");
            }
        }
    };

    std::vector<std::uint8_t> request;

    if (operationType == RBCPOperation::Read)
    {
        std::uint8_t pSize = extractSize();

        request = Bytes::composeByteVec(true, rbcpVerType, rbcpCmdRd, rbcpId, pSize, pAddr);
    }
    else // if (operationType == RBCPOperation::Write)
    {
        const std::vector<std::uint8_t>& pData = extractData();

        request = Bytes::composeByteVec(true, rbcpVerType, rbcpCmdWr, rbcpId, static_cast<std::uint8_t>(pData.size()), pAddr);
        Bytes::appendToByteVec(request, pData);
    }

    int writeAttemptCnt = 0;

    for (;;)
    {
        std::uint8_t& currentRbcpId = request[2];

        currentRbcpId = ++rbcpId;   //Increase RBCP message ID

        ++writeAttemptCnt;

        checkAndClearReadBuffer("before completing send operation");

        bool writeTimedOut = false;

        try
        {
            udpSocketWrapper.write(request, udpTimeout, writeTimedOut);
        }
        catch (const std::runtime_error&)
        {
            if (writeTimedOut && writeAttemptCnt <= udpRetransmitCnt)                       // cppcheck-suppress knownConditionTrueFalse
            {
                Logger::logWarning("Write timeout on UDP socket of " + getSelfDescription() + " (in " + functionName + "). Retry write...");
                continue;
            }
            else if (writeTimedOut)                                                         // cppcheck-suppress knownConditionTrueFalse
                throw std::runtime_error("Write timeout.");
            else
                throw;  //Rethrow for unknown non-timeout exceptions
        }

        int readAttemptCnt = 0;

        for (;;)
        {
            ++readAttemptCnt;

            const auto timeoutRefTime = std::chrono::steady_clock::now();

            while ((std::chrono::steady_clock::now() - timeoutRefTime) < udpTimeout && udpSocketWrapper.readBufferEmpty())
                std::this_thread::yield();

            if (udpSocketWrapper.readBufferEmpty())
            {
                if (readAttemptCnt <= udpRetransmitCnt)
                {
                    Logger::logWarning("Read timeout on UDP socket of " + getSelfDescription() + " (in " + functionName + "). Retry read...");
                    continue;
                }
                else if (writeAttemptCnt <= udpRetransmitCnt)
                {
                    Logger::logWarning("Read timeout on UDP socket of " + getSelfDescription() + " (in " + functionName + "). Retry write...");
                    break;
                }
                else
                    throw std::runtime_error("Read timeout.");
            }

            //Read response message
            const std::vector<std::uint8_t> response = udpSocketWrapper.read();

            //Check if responded message equals sent request

            if (response.size() < 8)
                throw std::runtime_error("Received invalid RBCP message.");

            const auto rbcpStatus = std::span<const std::uint8_t, 8>(response.begin(), 8);

            //Try to read again if "just" message ID is wrong as correct response message could be still pending
            if (rbcpStatus[2] != rbcpId)
            {
                if (readAttemptCnt <= udpRetransmitCnt)
                {
                    Logger::logWarning("RBCP message received on " + getSelfDescription() + " has wrong ID " +
                                       "(in " + functionName + "). Retry read...");
                    continue;
                }
                else if (writeAttemptCnt <= udpRetransmitCnt)
                {
                    Logger::logWarning("RBCP message received on " + getSelfDescription() + " has wrong ID " +
                                       "(in " + functionName + "). Retry write...");
                    break;
                }
                else
                    throw std::runtime_error("Received RBCP message has wrong ID.");
            }

            if (rbcpStatus[0] != rbcpVerType)
                throw std::runtime_error("Received RBCP message shows invalid RBCP version.");

            if ((rbcpStatus[1] & 0b10111110u) != 0b10001000u)
                throw std::runtime_error("Received RBCP message has invalid status byte.");

            std::bitset<8> statusBits(rbcpStatus[1]);

            if (statusBits[0])
                throw std::runtime_error("Received RBCP message signals RBCP bus error.");

            if (statusBits[6] != (operationType == RBCPOperation::Read))
                throw std::runtime_error("Received RBCP message R/W type does not match current operation.");

            if (rbcpStatus[3] != request[3])
            {
                throw std::runtime_error("Received RBCP message has size field mismatch. Size: " +
                                         std::to_string(request[3]) + " (expected), " +
                                         std::to_string(rbcpStatus[3]) + " (received).");
            }

            if (!std::equal(rbcpStatus.begin()+4, rbcpStatus.end(), request.begin()+4, request.end()))
            {
                const std::uint32_t expAddr = Bytes::composeUInt32(std::span<const std::uint8_t, 4>(request.begin()+4, 4));
                const std::uint32_t recAddr = Bytes::composeUInt32(std::span<const std::uint8_t, 4>(rbcpStatus.begin()+4, 4));

                throw std::runtime_error("Received RBCP message has address mismatch. Address: " +
                                         Bytes::formatHex(expAddr) + " (expected), " +
                                         Bytes::formatHex(recAddr) + " (received).");
            }

            const std::size_t expectedSize = ((operationType == RBCPOperation::Read) ? (extractSize() + 8) : request.size());

            if (response.size() != expectedSize)
            {
                throw std::runtime_error("Received RBCP message has invalid size. Size: " +
                                         std::to_string(expectedSize) + " (expected), " +
                                         std::to_string(response.size()) + " (received).");
            }

            const auto rbcpData = std::span<const std::uint8_t>(response.begin()+8, response.end());

            if (operationType == RBCPOperation::Write)
            {
                const std::vector<std::uint8_t>& pData = extractData();

                if (!std::equal(rbcpData.begin(), rbcpData.end(), pData.begin(), pData.end()))
                {
                    std::vector<std::uint8_t> recData(rbcpData.begin(), rbcpData.end());

                    throw std::runtime_error("Received RBCP message has invalid data. Data: " +
                                             Bytes::formatByteVec(pData) + " (expected), " +
                                             Bytes::formatByteVec(recData) + " (received).");
                }
            }

            checkAndClearReadBuffer("after completing receive operation");

            if (operationType == RBCPOperation::Read)
                return std::vector<std::uint8_t>(rbcpData.begin(), rbcpData.end());
            else
                return std::nullopt;

        } // read attempts loop

        if (writeAttemptCnt > udpRetransmitCnt)
            throw std::runtime_error("Exceeded number of write attempts without throwing exception. THIS SHOULD NEVER HAPPEN!");

    } // write attempts loop

    throw std::runtime_error("Reached end of function. THIS SHOULD NEVER HAPPEN!");
}
