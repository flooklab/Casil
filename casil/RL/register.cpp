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

#include <casil/RL/register.h>

#include <utility>

using casil::Layers::RL::Register;

/*!
 * \brief Constructor.
 *
 * Constructs LayerBase for Layer::RegisterLayer, forwarding \p pType, \p pName, \p pConfig and \p pRequiredConfig.
 *
 * Initializes the internal driver instance (for required access to \ref casil::Layers::HL "HL") from \p pDriver.
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pDriver %Driver instance to be used.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 */
Register::Register(std::string pType, std::string pName, HL::Driver& pDriver, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    LayerBase(Layer::RegisterLayer, std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig),
    driver(pDriver)
{
}
