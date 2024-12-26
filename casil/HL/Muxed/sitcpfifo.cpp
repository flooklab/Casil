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

#include <casil/HL/Muxed/sitcpfifo.h>

#include <casil/bytes.h>
#include <casil/TL/Muxed/sitcp.h>

#include <span>
#include <stdexcept>
#include <utility>

using casil::Layers::HL::SiTCPFifo;

CASIL_REGISTER_DRIVER_CPP(SiTCPFifo)
CASIL_REGISTER_DRIVER_ALIAS("sitcp_fifo")

//

/*!
 * \brief Constructor.
 *
 * \throws std::bad_cast If \p pInterface is not TL::SiTCP.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
SiTCPFifo::SiTCPFifo(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    MuxedDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig()),
    siTcpIntf(dynamic_cast<SiTCP&>(interface))  //Possible exception will be caught by macro-registered factory generator
{
}

//Public

/*!
 * \brief Register-like access to some functions.
 *
 * If \p pRegName is
 * - "RESET": calls reset() and returns 0.
 * - "VERSION": returns getVersion().
 * - "FIFO_SIZE": returns getFifoSize().
 *
 * \throws std::invalid_argument If \p pRegName is neither of "RESET", "VERSION" or "FIFO_SIZE".
 * \throws std::runtime_error If reset() throws \c std::runtime_error.
 *
 * \param pRegName One of the pseudo registers "RESET", "VERSION" or "FIFO_SIZE".
 * \return A value depending on the action for \p pRegName, see above.
 */
std::size_t SiTCPFifo::operator[](std::string_view pRegName)
{
    if (pRegName == "RESET")
    {
        reset();
        return 0;
    }
    else if (pRegName == "VERSION")
        return getVersion();
    else if (pRegName == "FIFO_SIZE")
        return getFifoSize();
    else
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available for SiTCP FIFO driver \"" + name + "\".");
}

//

/*!
 * \brief Reset the FIFO.
 *
 * See TL::SiTCP::resetFifo().
 *
 * \throws std::runtime_error If TL::SiTCP::resetFifo() throws \c std::runtime_error.
 */
void SiTCPFifo::reset()
{
    try
    {
        siTcpIntf.resetFifo();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear FIFO from SiTCP FIFO driver \"" + name + "\": " + exc.what());
    }
}

//

/*!
 * \brief Get the pseudo FIFO module version.
 *
 * \return Faked version of the non-existent firmware module.
 */
std::uint8_t SiTCPFifo::getVersion() const
{
    return pseudoVersion;
}

//

/*!
 * \brief Get the FIFO size in number of bytes.
 *
 * See TL::SiTCP::getFifoSize().
 *
 * \return Current size of the \e %SiTCP FIFO.
 */
std::size_t SiTCPFifo::getFifoSize() const
{
    return siTcpIntf.getFifoSize();
}

/*!
 * \brief Read the FIFO content as sequence of 32 bit unsigned integers.
 *
 * Extracts/removes the currently longest possible sequence of \c 4*N bytes from the FIFO,
 * from which it generates and then returns a sequence of \c N 32 bit unsigned integers,
 * assuming a little endian byte order.
 *
 * See also TL::SiTCP::getFifoData().
 *
 * \throws std::runtime_error If TL::SiTCP::getFifoData() or TL::SiTCP::getFifoSize() throw \c std::runtime_error.
 * \throws std::runtime_error If TL::SiTCP::getFifoData() fails to exactly return a multiple of 4 bytes (should not really happen).
 *
 * \return Longest sequence of 32 bit unsigned integers currently in the \e %SiTCP FIFO.
 */
std::vector<std::uint32_t> SiTCPFifo::getFifoData() const
{
    std::size_t retSize;
    std::vector<std::uint8_t> rawData;
    try
    {
        retSize = siTcpIntf.getFifoSize() / 4;
        rawData = siTcpIntf.getFifoData(retSize * 4);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("SiTCP FIFO driver \"" + name + "\" could not get FIFO data: " + exc.what());
    }

    if (rawData.size() != retSize * 4)
        throw std::runtime_error("SiTCP interface returned wrong number of FIFO bytes.");

    std::vector<std::uint32_t> retVal;
    retVal.reserve(retSize);

    for (std::size_t i = 0; i < retSize; ++i)
        retVal.push_back(Bytes::composeUInt32(std::span<const std::uint8_t, 4>(rawData.begin() + 4*i, 4), false));

    return retVal;
}

/*!
 * \brief Write a sequence of 32 bit unsigned integers to the FIFO.
 *
 * Creates a sequence of \c 4*N bytes from the \c N elements of \p pData and writes this sequence to
 * the FIFO by calling TL::SiTCP::write() using the special address TL::SiTCP::baseAddrDataLimit.
 * Each 32 bit unsigned integer will be represented as 4 bytes in little endian byte order.
 *
 * See also TL::SiTCP::getFifoData().
 *
 * \throws std::runtime_error If TL::SiTCP::write() throws \c std::runtime_error.
 *
 * \param pData Sequence of 32 bit unsigned integers to write to the \e %SiTCP FIFO as bytes.
 */
void SiTCPFifo::setFifoData(const std::vector<std::uint32_t>& pData) const
{
    const std::size_t retSize = pData.size() * 4;

    std::vector<std::uint8_t> bytes;
    bytes.reserve(retSize);

    for (std::size_t i = 0; i < pData.size(); ++i)
        for (std::uint8_t val : Bytes::composeByteVec(false, pData[i]))
            bytes.push_back(val);                                                                       //cppcheck-suppress useStlAlgorithm

    try
    {
        siTcpIntf.write(SiTCP::baseAddrDataLimit, bytes);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("SiTCP FIFO driver \"" + name + "\" could not write FIFO data: " + exc.what());
    }
}

//Private

/*!
 * \brief Initialize the driver by doing nothing.
 *
 * \return True.
 */
bool SiTCPFifo::initImpl()
{
    return true;
}

/*!
 * \brief Close the driver by doing nothing.
 *
 * \return True.
 */
bool SiTCPFifo::closeImpl()
{
    return true;
}
