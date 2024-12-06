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

#include <casil/layerbase.h>

#include <casil/logger.h>

#include <stdexcept>
#include <utility>

using casil::LayerBase;

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pLayer
 * \param pType
 * \param pName
 * \param pConfig
 * \param pRequiredConfig
 */
LayerBase::LayerBase(const Layer pLayer, std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    layer(pLayer),
    type(std::move(pType)),
    name(std::move(pName)),
    config(std::move(pConfig)),
    initialized(false),
    selfDescription("\"" + type + "\"-" +
                    (layer == Layer::TransferLayer ? "interface" : (layer == Layer::HardwareLayer ? "driver" : "register")) +
                    " instance \"" + name + "\"")
{
    if (!config.contains(pRequiredConfig, true))
        throw std::runtime_error("Incomplete/invalid configuration for " + getSelfDescription() + ".");
}

//Public

/*!
 * \brief Get the layer of this layer component.
 *
 * \todo Detailed doc
 *
 * \return
 */
LayerBase::Layer LayerBase::getLayer() const
{
    return layer;
}

/*!
 * \brief Get the type name of this layer component.
 *
 * \todo Detailed doc
 *
 * \return
 */
const std::string& LayerBase::getType() const
{
    return type;
}

/*!
 * \brief Get the instance name of this layer component.
 *
 * \todo Detailed doc
 *
 * \return
 */
const std::string& LayerBase::getName() const
{
    return name;
}

//

/*!
 * \brief Initialize this layer component.
 *
 * \todo Detailed doc
 *
 * \param pForce
 * \return
 */
bool LayerBase::init(const bool pForce)
{
    if (initialized && !pForce)
        return true;

    initialized = false;

    Logger::logDebug("Initializing " + getSelfDescription() + "...");

    if (!initImpl())
        return false;

    initialized = true;

    return true;
}

/*!
 * \brief Close ("uninitialize") this layer component.
 *
 * \todo Detailed doc
 *
 * \param pForce
 * \return
 */
bool LayerBase::close(const bool pForce)
{
    if (!initialized && !pForce)
        return true;

    Logger::logDebug("Closing " + getSelfDescription() + "...");

    if (!closeImpl())
        return false;

    initialized = false;

    return true;
}

//Protected

/*!
 * \brief Get a standard description of this layer component for logging purposes.
 *
 * \todo Detailed doc
 *
 * \return
 */
const std::string& LayerBase::getSelfDescription() const
{
    return selfDescription;
}
