/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
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

#ifndef CASIL_DEVICE_H
#define CASIL_DEVICE_H

#include <casil/layerbase.h>
#include <casil/HL/driver.h>
#include <casil/RL/register.h>
#include <casil/TL/interface.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace casil
{

/*!
 * \brief Configurable container class for interdependent layer components to interact with an arbitrary DAQ setup.
 *
 * Uses a basil YAML configuration document to automatically construct/configure all required components of all three layers
 * (\ref Layers::TL "TL" / \ref Layers::HL "HL" / \ref Layers::RL "RL").
 *
 * These components can be collectively initialized and then individually accessed and used through this class's interface.
 */
class Device
{
private:
    Device();                                                       ///< Constructor.

public:
    explicit Device(const boost::property_tree::ptree& pConfig);    ///< Constructor.
    explicit Device(const std::string& pConfig);                    ///< Constructor.
    Device(const Device&) = delete;                                 ///< Deleted copy constructor.
    Device(Device&&) = default;                                     ///< Default move constructor.
    virtual ~Device();                                              ///< Destructor.
    //
    Device& operator=(Device) = delete;                             ///< Deleted copy assignment operator.
    Device& operator=(Device&&) = delete;                           ///< Deleted move assignment operator.
    //
    LayerBase& operator[](std::string_view pName) const;            ///< Access one of the components from any layer.
    //
    TL::Interface& interface(std::string_view pName) const;         ///< Access one of the interface components from the transfer layer.
    HL::Driver& driver(std::string_view pName) const;               ///< Access one of the driver components from the hardware layer.
    RL::Register& reg(std::string_view pName) const;                ///< Access one of the register components from the register layer.
    //
    bool init(bool pForce = false);                                 ///< Initialize by initializing all components of all layers.
    bool close(bool pForce = false);                                ///< Close by closing all components of all layers.
    //
    bool loadRuntimeConfiguration(const std::map<std::string, std::string>& pConf) const;
                                                                    ///< Load additional runtime configuration data/values for the components.
    std::map<std::string, std::string> dumpRuntimeConfiguration() const;
                                                                    ///< Save current runtime configuration data/values of the components.

private:
    std::map<std::string, const std::unique_ptr<TL::Interface>, std::less<>> interfaces;    ///< Map of all interfaces with their names as keys.
    std::map<std::string, const std::unique_ptr<HL::Driver>, std::less<>> drivers;          ///< Map of all drivers with their names as keys.
    std::map<std::string, const std::unique_ptr<RL::Register>, std::less<>> registers;      ///< Map of all registers with their names as keys.
    //
    bool initialized;                                                                       ///< Initialized and not closed.
};

} // namespace casil

#endif // CASIL_DEVICE_H
