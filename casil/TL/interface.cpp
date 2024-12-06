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

#include <casil/TL/interface.h>

#include <casil/auxil.h>

#include <utility>

using casil::TL::Interface;

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pName
 * \param pConfig
 * \param pRequiredConfig
 */
Interface::Interface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    LayerBase(Layer::TransferLayer, std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig),
    queryDelay(config.getDbl("init.query_delay", 0.0)),
    queryDelayMicroSecs(Auxil::getChronoMicroSecs(queryDelay*1e-3))
{
    if (queryDelay < 0.0)
        throw std::runtime_error("Negative query delay set for " + getSelfDescription() + ".");
}
