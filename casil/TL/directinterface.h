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

#ifndef CASIL_LAYERS_TL_DIRECTINTERFACE_H
#define CASIL_LAYERS_TL_DIRECTINTERFACE_H

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
 * \brief Base class to derive from for interface components that \e directly connect to an \e independent hardware device.
 *
 * Use this class as base class for implementing a "direct" interface, i.e. an interface which connects to some stand-alone hardware
 * device that can work independently of other used hardware devices and that is usually only contolled by a single HL::Driver component.
 *
 * Note: This is in contrast to MuxedInterface, which should be used for FPGA hardware based on the basil bus,
 * as there will usually be \e multiple drivers controlling \e multiple firmware modules over the \e same interface.
 */
class DirectInterface : public Interface
{
public:
    DirectInterface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig); ///< Constructor.
    ~DirectInterface() override = default;                                                                          ///< Default destructor.
    //
    /*!
     * \brief Read from the interface.
     *
     * \param pSize Number of bytes to read.
     * \return Read bytes.
     */
    virtual std::vector<std::uint8_t> read(int pSize = -1) = 0;
    /*!
     * \brief Write to the interface.
     *
     * \param pData %Bytes to be written.
     */
    virtual void write(const std::vector<std::uint8_t>& pData) = 0;
    virtual std::vector<std::uint8_t> query(const std::vector<std::uint8_t>& pData, int pSize = -1) = 0;    ///< \brief Write a query to the
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

#endif // CASIL_LAYERS_TL_DIRECTINTERFACE_H
