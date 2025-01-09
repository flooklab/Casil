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

#ifndef CASIL_LAYERS_HL_DRIVER_H
#define CASIL_LAYERS_HL_DRIVER_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers
{

/*!
 * \brief Hardware layer: Drivers that control the connected devices/components.
 *
 * This is the second layer, which provides Driver components as the next higher-level abstraction
 * as compared to the interfaces in the transfer layer (TL). The drivers define and abstract protocols
 * needed for actually \e controlling the devices/components that are connected via the transfer layer.
 *
 * The drivers do not deal with the details of the transmission of those protocols, which is already taken care of by TL.
 * The idea is that the protocols can in principle be sent over any configured interface component, at least if the used
 * hardware can be connected to the host via different interfaces and no "special" functionality is needed. Another (obvious)
 * contraint is that only "direct"/"muxed" interfaces can be used for "direct"/"muxed" drivers (see Driver and TL::Interface).
 *
 * An even higher level abstraction is available for e.g. "muxed" drivers that expose access to \e hardware \e registers
 * of the respective firmware modules or any drivers that provide other functionality that can be operated on as if it
 * was a hardware register. This abstraction for individual such registers is provided by the register layer (RL).
 *
 * Note that hardware register access is also possible from the hardware layer using "register drivers" (see RegisterDriver).
 */
namespace HL
{

/*!
 * \brief Common base class for all driver components in the hardware layer (HL).
 *
 * This class provides very basic/limited functionality likely common to all drivers.
 * It is not intended to directly derive drivers from this class.
 *
 * Because of the \e direct/muxed interface split in the \ref Layers::TL "TL", drivers must be split into two such groups as well:
 * - "Direct" drivers: Drivers for independent/stand-alone hardware (not using the basil bus)
 * - "Muxed" drivers: Drivers for firmware modules running on FPGA hardware based on the basil bus
 *
 * Hence use either DirectDriver for implementing a \e direct driver or MuxedDriver
 * (or its RegisterDriver specialization) for implementing a \e muxed driver.
 */
class Driver : public LayerBase
{
public:
    Driver(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);  ///< Constructor.
    ~Driver() override = default;                                                                           ///< Default destructor.
    //
    virtual void reset();                                                                           ///< Reset the controlled device/module.
    //
    /*!
     * \brief Get driver-specific special data.
     *
     * \param pSize Potentially number of bytes to get (implementation-defined).
     * \param pAddrOffs Potentially data offset as number of bytes (implementation-defined).
     * \return Requested data as byte sequence.
     */
    virtual std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) = 0;
    /*!
     * \brief Set driver-specific special data.
     *
     * \param pData Data to be set as byte sequence.
     * \param pAddrOffs Potentially data offset as number of bytes (implementation-defined).
     */
    virtual void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) = 0;
    virtual void exec() = 0;                                                                        ///< Perform a driver-specific action.
    /*!
     * \brief Check if a driver-specific action has finished.
     *
     * \return True if finished.
     */
    virtual bool isDone() = 0;
    //
    //TODO could something be added here to enable direct C++ FunctionalRegister-functionality without python wrapper in between?

private:
    /*!
     * \brief Perform driver-specific initialization logic for init().
     *
     * \copydetails LayerBase::initImpl()
     */
    bool initImpl() override = 0;
    /*!
     * \brief Perform driver-specific closing logic for init().
     *
     * \copydetails LayerBase::closeImpl()
     */
    bool closeImpl() override = 0;
};

} // namespace HL

} // namespace Layers

namespace HL = Layers::HL;

} // namespace casil

#endif // CASIL_LAYERS_HL_DRIVER_H
