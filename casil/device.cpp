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

#include <casil/device.h>

#include <casil/auxil.h>
#include <casil/layerconfig.h>
#include <casil/layerfactory.h>
#include <casil/logger.h>

#include <boost/property_tree/ptree.hpp>

#include <utility>
#include <set>
#include <stdexcept>

using casil::Device;

using casil::LayerBase;
using casil::TL::Interface;
using casil::HL::Driver;
using casil::RL::Register;

/*!
 * \brief Constructor.
 *
 * Base constructor that should be called from the other constructors.
 */
Device::Device() :
    initialized(false)
{
}

//

/*!
 * \brief Constructor.
 *
 * Reads the configurations for all layer components from \p pConfig (\ref Layers::TL "TL" / \ref Layers::HL "HL" /
 * \ref Layers::RL "RL" from "transfer_layer" / "hw_drivers" / "registers" sections) and constructs all of those
 * components accordingly using the LayerFactory. First constructs all interfaces, then all drivers and then all registers.
 *
 * The generally mandatory configuration keys "name" and "type" (and "interface" for \ref Layers::HL "HL" and "hw_driver"
 * for \ref Layers::RL "RL") are separately processed and therefore stripped from the individual configurations before
 * they are passed as LayerConfig to the LayerFactory (and eventually to the component constructors).
 *
 * \throws std::runtime_error If mandatory parts are missing from \p pConfig or construction of a component fails.
 *
 * \param pConfig %Device configuration tree as loaded from a basil YAML configuration file via Auxil::propertyTreeFromYAML().
 */
Device::Device(const boost::property_tree::ptree& pConfig) :
    Device()
{
    using boost::property_tree::ptree;
    using boost::property_tree::ptree_bad_path;

    try
    {
        const ptree& tlConf = pConfig.get_child("transfer_layer");
        const ptree& hlConf = pConfig.get_child("hw_drivers");
        const ptree& rlConf = pConfig.get_child("registers");

        //Make sure that component names are unique
        std::set<std::string> componentNames;

        for (auto [key, intfConf] : tlConf)
        {
            const std::string intfName = intfConf.get_child("name").data();
            const std::string intfType = intfConf.get_child("type").data();

            intfConf.erase("name");
            intfConf.erase("type");

            if (componentNames.contains(intfName))
                throw std::runtime_error("Cannot create interface \"" + intfName + "\": The name is already used by another component.");
            else
                componentNames.insert(intfName);

            try
            {
                interfaces.emplace(intfName, LayerFactory::createInterface(intfType, intfName, LayerConfig(std::move(intfConf))));
            }
            catch (const std::runtime_error& exc)
            {
                throw std::runtime_error("Could not create interface \"" + intfName + "\": " + exc.what());
            }
        }

        for (auto [key, drvConf] : hlConf)
        {
            const std::string drvName = drvConf.get_child("name").data();
            const std::string drvType = drvConf.get_child("type").data();
            const std::string intfName = drvConf.get_child("interface").data();

            drvConf.erase("name");
            drvConf.erase("type");
            drvConf.erase("interface");

            if (componentNames.contains(drvName))
                throw std::runtime_error("Cannot create driver \"" + drvName + "\": The name is already used by another component.");
            else
                componentNames.insert(drvName);

            const auto it = interfaces.find(intfName);

            if (it == interfaces.end())
                throw std::runtime_error("No interface with name \"" + intfName + "\" defined.");

            try
            {
                Interface& intf = *(it->second);
                drivers.emplace(drvName, LayerFactory::createDriver(drvType, drvName, intf, LayerConfig(std::move(drvConf))));
            }
            catch (const std::runtime_error& exc)
            {
                throw std::runtime_error("Could not create driver \"" + drvName + "\": " + exc.what());
            }
        }

        for (auto [key, regConf] : rlConf)
        {
            const std::string regName = regConf.get_child("name").data();
            const std::string regType = regConf.get_child("type").data();
            const std::string drvName = regConf.get_child("hw_driver").data();

            regConf.erase("name");
            regConf.erase("type");
            regConf.erase("hw_driver");

            if (componentNames.contains(regName))
                throw std::runtime_error("Cannot create register \"" + regName + "\": The name is already used by another component.");
            else
                componentNames.insert(regName);

            const auto it = drivers.find(drvName);

            if (it == drivers.end())
                throw std::runtime_error("No driver with name \"" + drvName + "\" defined.");

            try
            {
                Driver& drv = *(it->second);
                registers.emplace(regName, LayerFactory::createRegister(regType, regName, drv, LayerConfig(std::move(regConf))));
            }
            catch (const std::runtime_error& exc)
            {
                throw std::runtime_error("Could not create register \"" + regName + "\": " + exc.what());
            }
        }
    }
    catch (const ptree_bad_path& exc)
    {
        throw std::runtime_error("Missing essential configuration key \"" + exc.path<ptree::path_type>().dump() + "\".");
    }
}

/*!
 * \brief Constructor.
 *
 * Calls Device(const boost::property_tree::ptree&) with the configuration tree loaded from \p pConfig via Auxil::propertyTreeFromYAML().
 *
 * \param pConfig YAML document with the device configuration.
 */
Device::Device(const std::string& pConfig) :
    Device(Auxil::propertyTreeFromYAML(pConfig))
{
}

/*!
 * \brief Destructor.
 *
 * Calls close() if still initialized (i.e. init() called but close() not called yet or failed).
 */
Device::~Device()
{
    //Make sure everything was closed before destruction
    if (initialized)
    {
        try
        {
            close(true);
        }
        catch (const std::runtime_error&)
        {
        }
    }
}

//Public

/*!
 * \brief Access one of the components from any layer.
 *
 * Returns a reference to the first component found with configured instance name \p pName
 * (same as component's "name" value from the YAML configuration tree).
 * Searches interfaces first, then drivers, then registers.
 *
 * \throws std::invalid_argument If no component with name \p pName was configured.
 *
 * \param pName Configured instance name of the requested component.
 * \return The LayerBase component with name \p pName.
 */
LayerBase& Device::operator[](const std::string_view pName) const
{
    const auto intfIt = interfaces.find(pName);
    if (intfIt != interfaces.end())
        return *(intfIt->second);

    const auto drvIt = drivers.find(pName);
    if (drvIt != drivers.end())
        return *(drvIt->second);

    const auto regIt = registers.find(pName);
    if (regIt != registers.end())
        return *(regIt->second);

    throw std::invalid_argument("No component with name \"" + std::string(pName) + "\".");
}

//

/*!
 * \brief Access one of the interface components from the transfer layer.
 *
 * Returns a reference to the \ref Layers::TL::Interface "TL::Interface" component with configured instance name \p pName
 * (same as component's "name" value from the YAML configuration tree).
 *
 * \throws std::invalid_argument If no interface with name \p pName was configured.
 *
 * \param pName Configured instance name of the requested component.
 * \return The \ref Layers::TL::Interface "TL::Interface" component with name \p pName.
 */
casil::TL::Interface& Device::interface(const std::string_view pName) const
{
    const auto it = interfaces.find(pName);
    if (it != interfaces.end())
        return *(it->second);
    else
        throw std::invalid_argument("No interface with name \"" + std::string(pName) + "\".");
}

/*!
 * \brief Access one of the driver components from the hardware layer.
 *
 * Returns a reference to the \ref Layers::HL::Driver "HL::Driver" component with configured instance name \p pName
 * (same as component's "name" value from the YAML configuration tree).
 *
 * \throws std::invalid_argument If no driver with name \p pName was configured.
 *
 * \param pName Configured instance name of the requested component.
 * \return The \ref Layers::HL::Driver "HL::Driver" component with name \p pName.
 */
casil::HL::Driver& Device::driver(const std::string_view pName) const
{
    const auto it = drivers.find(pName);
    if (it != drivers.end())
        return *(it->second);
    else
        throw std::invalid_argument("No driver with name \"" + std::string(pName) + "\".");
}

/*!
 * \brief Access one of the register components from the register layer.
 *
 * Returns a reference to the \ref Layers::RL::Register "RL::Register" component with configured instance name \p pName
 * (same as component's "name" value from the YAML configuration tree).
 *
 * \throws std::invalid_argument If no register with name \p pName was configured.
 *
 * \param pName Configured instance name of the requested component.
 * \return The \ref Layers::RL::Register "RL::Register" component with name \p pName.
 */
casil::RL::Register& Device::reg(const std::string_view pName) const
{
    const auto it = registers.find(pName);
    if (it != registers.end())
        return *(it->second);
    else
        throw std::invalid_argument("No register with name \"" + std::string(pName) + "\".");
}

//

/*!
 * \brief Initialize by initializing all components of all layers.
 *
 * Calls LayerBase::init() for every interface, then for every driver and then for every register.
 * \p pForce is forwarded for every component.
 *
 * Immediately returns true, instead, if already initialized, unless \p pForce is set.
 *
 * Skips remaining components, resets initialized state and returns false if LayerBase::init() fails for one component.
 *
 * Remembers the initialized state on success for all components. This state can be reset via close().
 *
 * \param pForce Ignore initialized state.
 * \return True if all components were/are successfully initialized.
 */
bool Device::init(const bool pForce)
{
    if (initialized && !pForce)
        return true;

    initialized = false;

    for (const auto& [key, intf] : interfaces)
        if (!intf->init(pForce))
            return false;

    for (const auto& [key, drv] : drivers)
        if (!drv->init(pForce))
            return false;

    for (const auto& [key, regter] : registers)
        if (!regter->init(pForce))
            return false;

    initialized = true;

    return true;
}

/*!
 * \brief Close by closing all components of all layers.
 *
 * Calls LayerBase::close() for every register, then for every driver and then for every interface.
 * \p pForce is forwarded for every component.
 *
 * Immediately returns true, instead, if already closed (i.e. unset initialized state), unless \p pForce is set.
 *
 * Skips remaining components and returns false if LayerBase::close() fails for one component.
 *
 * Unsets the initialized state (set by init()) on success for all components.
 *
 * \param pForce Ignore (not-)initialized state.
 * \return True if all components were/are successfully closed.
 */
bool Device::close(const bool pForce)
{
    if (!initialized && !pForce)
        return true;

    for (const auto& [key, regter] : registers)
        if (!regter->close(pForce))
            return false;

    for (const auto& [key, drv] : drivers)
        if (!drv->close(pForce))
            return false;

    for (const auto& [key, intf] : interfaces)
        if (!intf->close(pForce))
            return false;

    initialized = false;

    return true;
}

//

/*!
 * \brief Load additional runtime configuration data/values for the components.
 *
 * Loads the runtime configuration for every existing component that is specified in
 * \p pConf by calling LayerBase::loadRuntimeConfiguration(). This is done in order,
 * first for every interface, then for every driver and then for every register.
 * The keys in \p pConf determine the respective component names and the values
 * will be passed as arguments to LayerBase::loadRuntimeConfiguration().
 *
 * Skips remaining components and returns false if loading the configuration fails for one component.
 *
 * Elements in \p pConf that refer to non-existent components will be ignored.
 *
 * \param pConf Map of runtime configurations (as YAML documents) with the component names as keys.
 * \return If successful.
 */
bool Device::loadRuntimeConfiguration(const std::map<std::string, std::string>& pConf) const
{
    for (const auto& [key, intf] : interfaces)
        if (pConf.contains(key))
            if (!intf->loadRuntimeConfiguration(pConf.at(key)))
                return false;

    for (const auto& [key, drv] : drivers)
        if (pConf.contains(key))
            if (!drv->loadRuntimeConfiguration(pConf.at(key)))
                return false;

    for (const auto& [key, regter] : registers)
        if (pConf.contains(key))
            if (!regter->loadRuntimeConfiguration(pConf.at(key)))
                return false;

    //Warn about missing components
    for (const auto& it : pConf)
        if (!interfaces.contains(it.first) && !drivers.contains(it.first) && !registers.contains(it.first))
            Logger::logWarning("Did not load runtime configuration for component \"" + it.first + "\": No such component.");

    return true;
}

/*!
 * \brief Save current runtime configuration data/values of the components.
 *
 * Collects the components' runtime configurations by calling LayerBase::dumpRuntimeConfiguration()
 * for every register, then for every driver and then for every interface.
 * A map will be returned that gets filled with only the non-empty configurations.
 *
 * \throws std::runtime_error If LayerBase::dumpRuntimeConfiguration() throws \c std::runtime_error for one of the components.
 *
 * \return Map of non-empty runtime configurations (as YAML documents) with the component names as keys.
 */
std::map<std::string, std::string> Device::dumpRuntimeConfiguration() const
{
    std::map<std::string, std::string> tConf;

    try
    {
        for (const auto& [key, regter] : registers)
        {
            std::string tDump = regter->dumpRuntimeConfiguration();
            if (tDump != "")
                tConf[key] = std::move(tDump);
        }

        for (const auto& [key, drv] : drivers)
        {
            std::string tDump = drv->dumpRuntimeConfiguration();
            if (tDump != "")
                tConf[key] = std::move(tDump);
        }

        for (const auto& [key, intf] : interfaces)
        {
            std::string tDump = intf->dumpRuntimeConfiguration();
            if (tDump != "")
                tConf[key] = std::move(tDump);
        }
    }
    catch (const std::runtime_error&)
    {
        throw;
    }

    return tConf;
}
