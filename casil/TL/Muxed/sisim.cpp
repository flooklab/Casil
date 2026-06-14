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

#include <casil/TL/Muxed/sisim.h>

#include <casil/bytes.h>
#include <casil/TL/CommonImpl/tcpsocketwrapper.h>

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>

using casil::Layers::TL::SiSim;

CASIL_REGISTER_INTERFACE_CPP(SiSim)

//

/*!
 * \brief Constructor.
 *
 * Initializes the host name to connect to from the optional "init.host" string in \p pConfig (default: "localhost").
 *
 * Initializes the network port for the connection from the optional
 * "init.port" value in \p pConfig (unsigned integer type, default: 12345).
 *
 * Initializes the maximum number of retry attempts for establishing a connection (see init()) from
 * the optional "init.connect_retries" value in \p pConfig (unsigned integer type, default: 60).
 *
 * \throws std::runtime_error If "init.host" is empty.
 * \throws std::runtime_error If "init.port" is out of range (must be in <tt>(0, 65535]</tt>).
 *
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 */
SiSim::SiSim(std::string pName, LayerConfig pConfig) :
    MuxedInterface(typeName, std::move(pName), std::move(pConfig), LayerConfig()),
    hostName(config.getStr("init.host", "localhost")),
    port(config.getUInt("init.port", 12345)),
    maxConnectRetries(config.getUInt("init.connect_retries", 60)),
    socketWrapperPtr(std::make_unique<CommonImpl::TCPSocketWrapper>(hostName, port, "", ""))
{
    if (hostName == "")
        throw std::runtime_error("No address/hostname set for " + getSelfDescription() + ".");
    if (port == 0 || port > 65535)
        throw std::runtime_error("Invalid port number set for " + getSelfDescription() + ".");

    if (config.contains(LayerConfig::fromYAML("{init: {timeout: }}"), false))
    {
        logger.logWarning("The \"init.timeout\" setting is unsupported but set. "
                          "Please use the setting \"init.connect_retries\" instead.");
    }
}

/*!
 * \brief Default destructor.
 */
SiSim::~SiSim() = default;

//Public

/*!
 * \copybrief MuxedInterface::read()
 *
 * Serializes \p pAddr and \p pSize into a \ref casil::TL::SiSim::MessageType::ReadRequest "ReadRequest" (see createReadRequest())
 * for the simulation and writes that to the socket, effectively requesting a read of \p pSize bytes from the basil bus
 * at \p pAddr. Then reads the \ref casil::TL::SiSim::MessageType::ReadResponse "ReadResponse" from the socket and
 * deserializes it (see parseReadResponse()) and returns the contained data (which should be the \p pSize requested bytes).
 *
 * \internal See also CommonImpl::TCPSocketWrapper::write() and CommonImpl::TCPSocketWrapper::read(). \endinternal
 *
 * \throws std::runtime_error For negative \p pSize.
 * \throws std::runtime_error If the write or the read fail.
 * \throws std::runtime_error If the received response is not a valid \ref casil::TL::SiSim::MessageType::ReadResponse "ReadResponse".
 *
 * \copydetails MuxedInterface::read()
 */
std::vector<std::uint8_t> SiSim::read(const std::uint64_t pAddr, const int pSize)
{
    if (pSize < 0)
        throw std::runtime_error("Requested read size is invalid for " + getSelfDescription() + ".");

    try
    {
        socketWrapperPtr->write(createReadRequest(pAddr, static_cast<std::uint32_t>(pSize)));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to TCP socket for " + getSelfDescription() + ": " + exc.what());
    }

    try
    {
        const MessageType msgType = parseMessageType(socketWrapperPtr->read(1).at(0));
        if (msgType != MessageType::ReadResponse)
            throw std::runtime_error("Received message of unexpected message type from simulation (should have been \"ReadResponse\").");

        const std::uint32_t messageSize = parseMessageSize(socketWrapperPtr->read(4));

        return parseReadResponse(socketWrapperPtr->read(static_cast<int>(messageSize)));
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Could not determine message type for " + getSelfDescription() + ".");
    }
    catch (const std::invalid_argument& exc)
    {
        throw std::runtime_error("Could not determine message type or size for " + getSelfDescription() + ": " + exc.what());
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not read from TCP socket for " + getSelfDescription() + ": " + exc.what());
    }
}

/*!
 * \copybrief MuxedInterface::write()
 *
 * Serializes \p pAddr and \p pData into a \ref casil::TL::SiSim::MessageType::WriteRequest "WriteRequest" (see createWriteRequest())
 * for the simulation and writes that to the socket, effectively writing \p pData to the basil bus at \p pAddr.
 *
 * \internal See also CommonImpl::TCPSocketWrapper::write(). \endinternal
 *
 * \throws std::runtime_error If the write fails.
 *
 * \copydetails MuxedInterface::write()
 */
void SiSim::write(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    try
    {
        socketWrapperPtr->write(createWriteRequest(pAddr, pData));
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not write to TCP socket for " + getSelfDescription() + ": " + exc.what());
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
std::vector<std::uint8_t> SiSim::query(std::uint64_t, std::uint64_t, const std::vector<std::uint8_t>&, int)
{
    logger.logWarning("The query() function is not implemented for the SiSim interface (does nothing).");
    return {};
}

//

/*!
 * \copybrief MuxedInterface::readBufferEmpty()
 *
 * \throws std::runtime_error If checking the buffer size fails.
 *
 * \copydetails MuxedInterface::readBufferEmpty()
 */
bool SiSim::readBufferEmpty() const
{
    try
    {
        return socketWrapperPtr->readBufferEmpty();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not check read buffer size of TCP socket \"" + name + "\": " + exc.what());
    }
}

/*!
 * \copybrief MuxedInterface::clearReadBuffer()
 *
 * \throws std::runtime_error If clearing the buffer fails.
 */
void SiSim::clearReadBuffer()
{
    try
    {
        socketWrapperPtr->clearReadBuffer();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear read buffer of TCP socket \"" + name + "\": " + exc.what());
    }
}

//

/*!
 * \brief Create a write request to send to simulation.
 *
 * Serializes \p pAddr and \p pData into a payload byte sequence and prepends a general message header consisting
 * of a byte representing the MessageType and four bytes (big endian) that announce the payload size in bytes.
 *
 * Decode the message by stripping message type (see parseMessageType()) and payload size (see parseMessageSize())
 * and deserializing the payload via parseWriteRequest().
 *
 * \internal See also createMessage(). \endinternal
 *
 * \param pAddr Bus address.
 * \param pData %Bytes to be written.
 * \return <tt>{messageType, payloadSize[MSB], ... payloadSize[LSB], pAddr[MSB], ..., pAddr[LSB], pData[0], ..., pData[pData.size()-1]}</tt>.
 */
std::vector<std::uint8_t> SiSim::createWriteRequest(const std::uint64_t pAddr, const std::vector<std::uint8_t>& pData)
{
    std::vector<std::uint8_t> payload = Bytes::composeByteVec(true, pAddr);
    Bytes::appendToByteVec(payload, pData);
    return createMessage(MessageType::WriteRequest, payload);
}

/*!
 * \brief Create a read request to send to simulation.
 *
 * Serializes \p pAddr and \p pSize into a payload byte sequence and prepends a general message header consisting
 * of a byte representing the MessageType and four bytes (big endian) that announce the payload size in bytes.
 *
 * Decode the message by stripping message type (see parseMessageType()) and payload size (see parseMessageSize())
 * and deserializing the payload via parseReadRequest().
 *
 * \internal See also createMessage(). \endinternal
 *
 * \param pAddr Bus address.
 * \param pSize Number of bytes to read.
 * \return <tt>{messageType, payloadSize[MSB], ... payloadSize[LSB], pAddr[MSB], ..., pAddr[LSB], pSize[MSB], ..., pSize[LSB]}</tt>.
 */
std::vector<std::uint8_t> SiSim::createReadRequest(const std::uint64_t pAddr, const std::uint32_t pSize)
{
    return createMessage(MessageType::ReadRequest, Bytes::composeByteVec(true, pAddr, pSize));
}

/*!
 * \brief Create a read response to send back to interface.
 *
 * Serializes \p pData into a payload byte sequence (actually does nothing) and prepends a general message header
 * consisting of a byte representing the MessageType and four bytes (big endian) that announce the payload size in bytes.
 *
 * Decode the message by stripping message type (see parseMessageType()) and payload size (see parseMessageSize())
 * and deserializing the payload via parseReadResponse() (which actually does nothing either).
 *
 * \internal See also createMessage(). \endinternal
 *
 * \param pData %Bytes to be sent.
 * \return <tt>{messageType, payloadSize[MSB], ... payloadSize[LSB], pData[0], ..., pData[pData.size()-1]}</tt>.
 */
std::vector<std::uint8_t> SiSim::createReadResponse(const std::vector<std::uint8_t>& pData)
{
    return createMessage(MessageType::ReadResponse, pData);
}

//

/*!
 * \brief Parse a write request as received from interface.
 *
 * Deserializes the \e payload \p pData (excluding type and size header)
 * of a received message as previously serialized by createWriteRequest().
 *
 * \throws std::invalid_argument If \p pData is not at least eight bytes long.
 *
 * \param pData Received message payload (i.e. excluding type and size header).
 * \return Targeted bus address and bytes to be written as a tuple.
 */
std::tuple<std::uint64_t, std::vector<std::uint8_t>> SiSim::parseWriteRequest(const std::vector<std::uint8_t>& pData)
{
    if (pData.size() < 8)
        throw std::invalid_argument("Invalid payload size for a write request: Must be at least eight bytes long.");

    return {Bytes::composeUInt64(std::span<const std::uint8_t, 8>(pData.begin(), 8), true),
            std::vector<std::uint8_t>(pData.begin()+8, pData.end())};
}

/*!
 * \brief Parse a read request as received from interface.
 *
 * Deserializes the \e payload \p pData (excluding type and size header)
 * of a received message as previously serialized by createReadRequest().
 *
 * \throws std::invalid_argument If \p pData is not twelve bytes long.
 *
 * \param pData Received message payload (i.e. excluding type and size header).
 * \return Targeted bus address and number of bytes to read as a tuple.
 */
std::tuple<std::uint64_t, std::uint32_t> SiSim::parseReadRequest(const std::vector<std::uint8_t>& pData)
{
    if (pData.size() != 12)
        throw std::invalid_argument("Invalid payload size for a read request: Must be twelve bytes long.");

    return {Bytes::composeUInt64(std::span<const std::uint8_t, 8>(pData.begin(), 8), true),
            Bytes::composeUInt32(std::span<const std::uint8_t, 4>(pData.begin()+8, 4), true)};
}

/*!
 * \brief Parse a read response as received from simulation.
 *
 * Deserializes the \e payload \p pData (excluding type and size header)
 * of a received message as previously serialized by createReadResponse().
 *
 * Note: This function does not actually do anything and just returns \p pData, because createReadResponse()
 *       does not do anything either (except adding the message header, of course).
 *
 * \param pData Received message payload (i.e. excluding type and size header).
 * \return Requested byte sequence.
 */
std::vector<std::uint8_t> SiSim::parseReadResponse(const std::vector<std::uint8_t>& pData)
{
    return pData;
}

//

/*!
 * \brief Parse message type from first message byte.
 *
 * Casts \p pTypeByte to MessageType.
 *
 * \internal Inverse operation to createMessageTypeByte(). \endinternal
 *
 * \throws std::invalid_argument If \p pTypeByte is out of range for MessageType.
 *
 * \param pTypeByte First message byte.
 * \return Type of the received message.
 */
SiSim::MessageType SiSim::parseMessageType(const std::uint8_t pTypeByte)
{
    if (pTypeByte >= 3)
        throw std::invalid_argument("Invalid value for message type byte.");

    return static_cast<MessageType>(pTypeByte);
}

//

/*!
 * \brief Parse payload size from message size bytes.
 *
 * Interprets the four bytes in \p pData as a 32 bit unsigned integer in big endian byte order.
 *
 * \internal Inverse operation to createMessageTypeByte(). \endinternal
 *
 * \throws std::invalid_argument If \p pData is not four bytes long.
 *
 * \param pData The four payload size bytes of the received message (second through fifth byte).
 * \return Payload size of the received message.
 */
std::uint32_t SiSim::parseMessageSize(const std::vector<std::uint8_t>& pData)
{
    if (pData.size() != 4)
        throw std::invalid_argument("Invalid number of message size bytes: Must be four bytes.");

    return Bytes::composeUInt32(pData, true);
}

//Private

/*!
 * \brief Create message to send between interface/simulation or vice versa.
 *
 * Takes the already serialized message payload \p pData and prepends a general message header consisting of a byte
 * representing the message type \p pType and four bytes (big endian) that announce the size of \p pData in bytes.
 *
 * For decoding of a received message, use parseMessageType() to get the message type from the first
 * byte and parseMessageSize() to get the size of the following payload from the next four bytes.
 *
 * \param pType Intended type of the message to be sent (in agreement with the serialized payload format of \p pData).
 * \param pData Serialized payload to be sent.
 * \return <tt>{messageType, payloadSize[MSB], ... payloadSize[LSB], pData[0], ..., pData[pData.size()-1]}</tt>.
 */
std::vector<std::uint8_t> SiSim::createMessage(const MessageType pType, const std::vector<std::uint8_t>& pData)
{
    std::vector<std::uint8_t> retVal {createMessageTypeByte(pType)};
    Bytes::appendToByteVec(retVal, createMessageSizeSeq(pData.size()));
    Bytes::appendToByteVec(retVal, pData);
    return retVal;
}

/*!
 * \brief Create first message byte for a certain type.
 *
 * Casts \p pType to a raw byte.
 *
 * Inverse operation to parseMessageType().
 *
 * \param pType Intended type of the message to be sent.
 * \return First message byte.
 */
std::uint8_t SiSim::createMessageTypeByte(const MessageType pType)
{
    return std::to_underlying(pType);
}

/*!
 * \brief Create message size bytes according to payload size.
 *
 * Returns a four byte long sequence in big endian byte order that represents \p pSize.
 *
 * Inverse operation to parseMessageSize().
 *
 * \param pSize Intended payload size for the message to be sent.
 * \return The four payload size bytes (second through fifth byte of the whole message).
 */
std::vector<std::uint8_t> SiSim::createMessageSizeSeq(const std::uint32_t pSize)
{
    return Bytes::composeByteVec(true, pSize);
}

//

/*!
 * \copybrief MuxedInterface::initImpl()
 *
 * Resolves the configured host name and connects the socket to this endpoint via the configured port.
 * Each connection attempt is cancelled after a timeout of one second. If the retry count from the
 * component configuration is set to a non-zero value (see SiSim(); default: 60), the connection
 * attempt will be repeated maximally that many times with an additional one second delay in
 * between each attempt, until it succeeds. This gives the simulator some time to set up.
 *
 * \note Requires IO context threads to be running already (see ASIO::ioContextThreadsRunning()).
 *
 * \return True if successful.
 */
bool SiSim::initImpl()
{
    //Try to connect a few times to wait for the simulator to set up
    for (std::uint64_t i = 0; i <= maxConnectRetries; ++i)
    {
        logger.logDebug("Trying to connect to simulator...");

        try
        {
            socketWrapperPtr->init(std::chrono::milliseconds{1000});
            return true;
        }
        catch (const std::runtime_error& exc)
        {
            logger.logWarning(std::string("Could not connect socket: ") + exc.what() + " Retry...");
            std::this_thread::sleep_for(std::chrono::milliseconds{1000});
        }
    }
    logger.logError("Could not connect to simulation server (reached maximum retry count).");
    return false;
}

/*!
 * \copybrief MuxedInterface::closeImpl()
 *
 * Disconnects the socket.
 *
 * \return True if successful.
 */
bool SiSim::closeImpl()
{
    try
    {
        socketWrapperPtr->close();
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not close socket connection: ") + exc.what());
        return false;
    }

    return true;
}
