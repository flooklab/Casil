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
*/

#ifndef CASIL_LAYERS_HL_METADRIVER_H
#define CASIL_LAYERS_HL_METADRIVER_H

#include <casil/HL/muxeddriver.h>

#include <casil/layerconfig.h>

#include <string>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief Specialization for principally MuxedDriver components that indirectly control their
 *        actual firmware module counterpart on a meta level by means of a backend driver.
 *
 * Use this class as base class for implementing a "meta driver", i.e. a muxed driver which does, however, \e not
 * directly control a firmware module but instead uses / relies on \e another muxed driver to control the actual
 * firmware module in order to (potentially) accomplish more complex tasks. In this way these more complex tasks
 * can be made available to the user on a higher abstraction level. This chained/nested concept is mainly intended
 * for controlling "third party" devices such as further on-board ICs that are connected to the FPGA and can be
 * controlled by, for instance, some kind of bus/protocol that can be driven by one of the firmware modules.
 *
 * For example, there could be an ADC to be controlled via SPI and a firmware with a corresponding SPI module.
 * Instead of always having to manually take care of the ADC protocol (i.e. control commands and associated responses)
 * there could be a dedicated meta driver for this specific ADC (providing simple configuration and sampling functions),
 * which would then just have to be configured with the correct SPI driver as its backend driver.
 *
 * Note that a meta driver can in principle also use another meta driver as backend driver,
 * which even allows for a very clean functional abstraction of much more complex setups.
 *
 * Also note that a meta driver likely requires a very specific backend driver \e type and the configuration
 * then also only provides the freedom of assigning arbitrary \e instances of the same driver type.
 */
class MetaDriver : public MuxedDriver
{
public:
    MetaDriver(std::string pType, std::string pName, MuxedDriver& pBackendDriver, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
                                        ///< Constructor.
    ~MetaDriver() override = default;   ///< Default destructor.

private:
    /*!
     * \brief Initialize the backend drivers.
     *
     * \copydetails MuxedDriver::initImpl()
     */
    bool initImpl() override = 0;
    /*!
     * \brief Close the backend drivers.
     *
     * \copydetails MuxedDriver::closeImpl()
     */
    bool closeImpl() override = 0;

protected:
    MuxedDriver& backendDriver;         ///< The driver instance to be used as actual/backend driver.
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_METADRIVER_H
