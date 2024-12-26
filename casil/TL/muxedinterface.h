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
*/

#ifndef CASIL_LAYERS_TL_MUXEDINTERFACE_H
#define CASIL_LAYERS_TL_MUXEDINTERFACE_H

#include <casil/TL/interface.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers
{

namespace TL
{

/*!
 * \brief Base class to derive from for interface components that connect to an FPGA endpoint running the basil bus and firmware modules.
 *
 * Use this class as base class for implementing a "muxed" interface, i.e. an interface which enables to control different
 * \e addressable parts of the same hardware device. This would typically be an interface to connect to FPGA hardware based
 * on the basil bus, which allows multiple drivers to control multiple (dependent) firmware modules at different bus addresses.
 *
 * Note: This is in contrast to DirectInterface, which should be used for hardware that can work \e independently
 * of other used hardware devices and that is usually only contolled by a single HL::Driver component.
 */
class MuxedInterface : public Interface
{
public:
    MuxedInterface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);  ///< Constructor.
    ~MuxedInterface() override = default;                                                                           ///< Default destructor.
    //
    /*!
     * \brief Read from the interface.
     *
     * \param pAddr Bus address.
     * \param pSize Number of bytes to read.
     * \return Read bytes.
     */
    virtual std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) = 0;
    /*!
     * \brief Write to the interface.
     *
     * \param pAddr Bus address.
     * \param pData %Bytes to be written.
     */
    virtual void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) = 0;
    virtual std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                            const std::vector<std::uint8_t>& pData, int pSize = -1) = 0;    ///< \brief Write a query to the
                                                                                                            ///  interface and read the response.
    //
    bool readBufferEmpty() const override = 0;
    void clearReadBuffer() override = 0;

private:
    bool initImpl() override = 0;
    bool closeImpl() override = 0;
};

} // namespace TL

} // namespace Layers

} // namespace casil

#endif // CASIL_LAYERS_TL_MUXEDINTERFACE_H
