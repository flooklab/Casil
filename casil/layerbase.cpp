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

#include <casil/layerbase.h>

#include <casil/auxil.h>
#include <casil/logger.h>

#include <boost/property_tree/ptree.hpp>

#include <stdexcept>
#include <utility>

using casil::LayerBase;

/*!
 * \brief Constructor.
 *
 * Constructs a layer component as a component of layer \p pLayer with registered component type name \p pType
 * and configured instance name \p pName with component type-specific configuration \p pConfig.
 *
 * Checks that \p pRequiredConfig is "contained" in \p pConfig (see LayerConfig::contains()).
 *
 * \throws std::runtime_error If \p pConfig is incomplete/invalid because \p pRequiredConfig is not contained.
 *
 * \param pLayer %Layer that this component belongs to.
 * \param pType Name of the component type (as registered to the LayerFactory).
 * \param pName Name of the instance (as e.g. used in Device).
 * \param pConfig Instance-specific component configuration.
 * \param pRequiredConfig Reference configuration required for the components of type \p pType.
 */
LayerBase::LayerBase(const Layer pLayer, std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    layer(pLayer),
    type(std::move(pType)),
    name(std::move(pName)),
    config(std::move(pConfig)),
    initialized(false),
    logger(*this),  //Note: needs/accesses members layer, type and name
    selfDescription("\"" + type + "\"-" +
                    (layer == Layer::TransferLayer ? "interface" : (layer == Layer::HardwareLayer ? "driver" : "register")) +
                    " instance \"" + name + "\"")
{
    if (!config.contains(pRequiredConfig, true))
    {
        throw std::runtime_error("Incomplete/invalid configuration for " + getSelfDescription() + ". Passed configuration:\n" +
                                 config.toString() + "Required configuration:\n" + pRequiredConfig.toString());
    }
}

//Public

/*!
 * \brief Get the layer of this layer component.
 *
 * \return %Layer that this component belongs to.
 */
LayerBase::Layer LayerBase::getLayer() const
{
    return layer;
}

/*!
 * \brief Get the type name of this layer component.
 *
 * \return Name of the component type.
 */
const std::string& LayerBase::getType() const
{
    return type;
}

/*!
 * \brief Get the instance name of this layer component.
 *
 * \return Name of the component instance.
 */
const std::string& LayerBase::getName() const
{
    return name;
}

//

/*!
 * \brief Initialize this layer component.
 *
 * Initializes by performing component-specific initialization logic. Returns false if this fails and true else.
 *
 * Immediately returns true if already initialized, unless \p pForce is set.
 *
 * Remembers the initialized state on success (gets unset otherwise).
 *
 * \internal The component-specific logic is to be implemented by initImpl(). \endinternal
 *
 * \param pForce Ignore initialized state.
 * \return True if already or successfully initialized.
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
 * Closes by performing component-specific closing logic. Returns false if this fails and true else.
 *
 * Immediately returns true if already closed (i.e. unset initialized state), unless \p pForce is set.
 *
 * Unsets the initialized state (set by init()) on success.
 *
 * \internal The component-specific logic is to be implemented by closeImpl(). \endinternal
 *
 * \param pForce Ignore (not-)initialized state.
 * \return True if already or successfully closed.
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

//

/*!
 * \brief Load additional, component-specific configuration data/values.
 *
 * If the component implements certain stateful logic that is believed to be worth saving/loading
 * (see dumpRuntimeConfiguration(), which then returns more than an empty string), this function
 * may be used to update that state with information from the YAML document \p pConf.
 * The document structure and contained data are of course component-specific and
 * must be compatible to what is obtainable by dumpRuntimeConfiguration().
 *
 * If \p pConf is an empty string an empty runtime configuration will be applied.
 * Whether this does something interesting (such as resetting the configuration)
 * or nothing depends on the component-specific implementation.
 *
 * \internal The component-specific logic is to be implemented by loadRuntimeConfImpl(). \endinternal
 *
 * Note: It is assumed that both functions (this one and dumpRuntimeConfiguration())
 * are only used when the component is initialized.
 *
 * \param pConf Desired runtime configuration as a YAML document.
 * \return If successful.
 */
bool LayerBase::loadRuntimeConfiguration(const std::string& pConf)
{
    try
    {
        boost::property_tree::ptree tTree = Auxil::propertyTreeFromYAML(pConf);
        return loadRuntimeConfImpl(std::move(tTree));
    }
    catch (const std::runtime_error& exc)
    {
        logger.logError(std::string("Could not load runtime configuration: ") + exc.what());
        return false;
    }
}

/*!
 * \brief Save current state of component-specific configuration data/values.
 *
 * If the component implements certain stateful logic that is believed to be worth saving/loading,
 * this function may return that currently held information as a YAML document.
 * The document structure and contained data are of course component-specific.
 *
 * An empty string will be returned if the component does not implement this or if it does not currently hold such data.
 * \internal Refer also to LayerBase::dumpRuntimeConfImpl() and Auxil::propertyTreeToYAML(). \endinternal
 *
 * The saved runtime configuration or some altered configuration with the same format/structure
 * can be applied to the component using the complementary function loadRuntimeConfiguration().
 *
 * \internal The component-specific logic is to be implemented by dumpRuntimeConfImpl(). \endinternal
 *
 * Note: It is assumed that both functions are only used when the component is initialized.
 *
 * \throws std::runtime_error If assembling the configuration or generation of the YAML document fails.
 *
 * \return Current runtime configuration as a YAML document.
 */
std::string LayerBase::dumpRuntimeConfiguration() const
{
    try
    {
        boost::property_tree::ptree tTree = dumpRuntimeConfImpl();
        return Auxil::propertyTreeToYAML(tTree);
    }
    catch (const std::runtime_error& exc)
    {
        throw std::runtime_error("Could not dump runtime configuration for " + getSelfDescription() + ": " + exc.what());
    }
}

//Protected

/*!
 * \brief Get a standard description of this layer component for logging purposes.
 *
 * Returns a string like e.g. <tt>"TYPE"-interface instance "NAME"</tt> for an interface.
 *
 * \return Component description including layer, type name and instance name.
 */
const std::string& LayerBase::getSelfDescription() const
{
    return selfDescription;
}

//Private

/*!
 * \brief Perform component-specific loading of runtime configuration.
 *
 * Does nothing, just returns true (override for specific components if applicable).
 *
 * \param pConf Desired runtime configuration tree.
 * \return If successful.
 */
bool LayerBase::loadRuntimeConfImpl(boost::property_tree::ptree&&)
{
    return true;
}

/*!
 * \brief Perform component-specific saving of runtime configuration.
 *
 * Does nothing, just returns an empty tree (override for specific components if applicable).
 *
 * \return Current runtime configuration tree.
 */
boost::property_tree::ptree LayerBase::dumpRuntimeConfImpl() const
{
    return boost::property_tree::ptree();
}
