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

#include <casil/HL/driver.h>

#include <utility>

using casil::HL::Driver;

/*!
 * \brief Constructor.
 *
 * Constructs LayerBase for Layer::HardwareLayer, forwarding \p pType, \p pName, \p pConfig and \p pRequiredConfig.
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 */
Driver::Driver(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    LayerBase(Layer::HardwareLayer, std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig)
{
}

//Public

/*!
 * \brief Reset the controlled device/module.
 *
 * Does nothing (override for specific drivers if needed).
 */
void Driver::reset()
{
    //(sic!)
}
