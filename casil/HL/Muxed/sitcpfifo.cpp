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
 * \todo Detailed doc
 *
 * \param pName
 * \param pInterface
 * \param pConfig
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
 * \todo Detailed doc
 *
 * \param pRegName
 * \return
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
        throw std::invalid_argument("The register \"" + std::string(pRegName) + "\" is not available for SiTcp FIFO driver \"" + name + "\".");
}

//

/*!
 * \brief Reset the FIFO.
 *
 * \todo Detailed doc
 */
void SiTCPFifo::reset()
{
    try
    {
        siTcpIntf.resetFifo();
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not clear FIFO from SiTcp FIFO driver \"" + name + "\": " + exc.what());
    }
}

//

/*!
 * \brief Get the pseudo FIFO module version.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::uint8_t SiTCPFifo::getVersion() const
{
    return pseudoVersion;
}

//

/*!
 * \brief Get the FIFO size in number of bytes.
 *
 * \todo Detailed doc
 *
 * \return
 */
std::size_t SiTCPFifo::getFifoSize() const
{
    return siTcpIntf.getFifoSize();
}

/*!
 * \brief Read the FIFO content as sequence of 32 bit unsigned integers.
 *
 * \todo Detailed doc
 *
 * \return
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
        throw std::runtime_error("SiTcp FIFO driver \"" + name + "\" could not get FIFO data: " + exc.what());
    }

    if (rawData.size() != retSize * 4)
        throw std::runtime_error("SiTcp interface returned wrong number of FIFO bytes.");

    std::vector<std::uint32_t> retVal;
    retVal.reserve(retSize);

    for (std::size_t i = 0; i < retSize; ++i)
        retVal.push_back(Bytes::composeUInt32(std::span<const std::uint8_t, 4>(rawData.begin() + 4*i, 4), false));

    return retVal;
}

/*!
 * \brief Write a sequence of 32 bit unsigned integers to the FIFO.
 *
 * \todo Detailed doc
 *
 * \param pData
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
        throw std::runtime_error("SiTcp FIFO driver \"" + name + "\" could not write FIFO data: " + exc.what());
    }
}

//Private

/*!
 * \brief Initialize the driver by doing nothing.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool SiTCPFifo::initImpl()
{
    return true;
}

/*!
 * \brief Close the driver by doing nothing.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool SiTCPFifo::closeImpl()
{
    return true;
}
