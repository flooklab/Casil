/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#ifndef CASIL_LAYERS_TL_INTERFACE_H
#define CASIL_LAYERS_TL_INTERFACE_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <chrono>
#include <string>

namespace casil
{

namespace Layers
{

/*!
 * \brief Transfer layer: Interfaces that connect the Casil host to its devices/components.
 *
 * This is the first, most low-level layer, which provides Interface components as a means of abstracting access to / providing
 * \e communication with the hardware in question. Actual protocols for \e controlling the connected devices/components are
 * not included in this layer, just the low-level protocols needed for the plain communication (e.g. Serial, SiTCP, etc.).
 *
 * The idea is that the next layer (hardware layer (HL)) provides such "controlling protocols" and can in principle be
 * configured and used independently of the transfer layer components configured for communication, at least if the used
 * hardware can be connected to the host via different interfaces and no "special" functionality is needed. Another (obvious)
 * contraint is that only "direct"/"muxed" interfaces can be used for "direct"/"muxed" drivers (see Interface and HL::Driver).
 */
namespace TL
{

/*!
 * \brief Common base class for all interface components in the transfer layer (TL).
 *
 * This class provides very basic/limited functionality likely common to all interfaces.
 * It is not intended to directly derive interfaces from this class.
 *
 * Interfaces are split into two different groups that slightly differ in their needed API:
 * - "Direct" interfaces: Interfaces for independent/stand-alone hardware (not using the basil bus)
 * - "Muxed" interfaces: Interfaces for FPGA hardware based on the basil bus
 *
 * Hence use either of DirectInterface or MuxedInterface as base class for implementing an interface.
 */
class Interface : public LayerBase
{
public:
    Interface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);   ///< Constructor.
    ~Interface() override = default;                                                                            ///< Default destructor.
    //
    /*!
     * \brief Check if the read buffer is empty.
     *
     * \return True if read buffer is empty.
     */
    virtual bool readBufferEmpty() const = 0;
    virtual void clearReadBuffer() = 0;         ///< Clear the current contents of the read buffer.

private:
    /*!
     * \brief Perform interface-specific initialization logic for init().
     *
     * \copydetails LayerBase::initImpl()
     */
    bool initImpl() override = 0;
    /*!
     * \brief Perform interface-specific closing logic for init().
     *
     * \copydetails LayerBase::closeImpl()
     */
    bool closeImpl() override = 0;

protected:
    const double queryDelay;                    ///< Configured delay value for query operations (between write and read) in milliseconds.
    const std::chrono::microseconds queryDelayMicroSecs;    ///< Rounded chrono version of queryDelay.
};

} // namespace TL

} // namespace Layers

namespace TL = Layers::TL;

} // namespace casil

#endif // CASIL_LAYERS_TL_INTERFACE_H
