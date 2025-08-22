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

#ifndef CASIL_LAYERBASE_H
#define CASIL_LAYERBASE_H

#include <casil/contextuallogger.h>
#include <casil/layerconfig.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <cstdint>
#include <string>

namespace casil
{

/*!
 * \brief Common top-level base class that connects the different layers and layer components of the basil layer structure.
 *
 * All components from the \ref Layers::TL "TL", \ref Layers::HL "HL" and \ref Layers::RL "RL" layers are ultimately derived from this class.
 *
 * It is not intended to directly derive components from this class.
 * See the classes next in the hierarchy (in the Layers namespace) for how to implement
 * components for the specific layers (transfer layer: \ref Layers::TL::Interface "TL::Interface",
 * hardware layer: \ref Layers::HL::Driver "HL::Driver", register layer: \ref Layers::RL::Register "RL::Register").
 */
class LayerBase
{
public:
    enum class Layer : std::uint8_t;

public:
    LayerBase(Layer pLayer, std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
                                                                ///< Constructor.
    LayerBase(const LayerBase&) = delete;                       ///< Deleted copy constructor.
    LayerBase(LayerBase&&) = default;                           ///< Default move constructor.
    virtual ~LayerBase() = default;                             ///< Default destructor.
    //
    LayerBase& operator=(LayerBase) = delete;                   ///< Deleted copy assignment operator.
    LayerBase& operator=(LayerBase&&) = delete;                 ///< Deleted move assignment operator.
    //
    Layer getLayer() const;                                     ///< Get the layer of this layer component.
    const std::string& getType() const;                         ///< Get the type name of this layer component.
    const std::string& getName() const;                         ///< Get the instance name of this layer component.
    //
    bool init(bool pForce = false);                             ///< Initialize this layer component.
    bool close(bool pForce = false);                            ///< Close ("uninitialize") this layer component.
    //
    bool loadRuntimeConfiguration(const std::string& pConf);    ///< Load additional, component-specific configuration data/values.
    std::string dumpRuntimeConfiguration() const;               ///< Save current state of component-specific configuration data/values.

protected:
    const std::string& getSelfDescription() const;              ///< Get a standard description of this layer component for logging purposes.

private:
    /*!
     * \brief Perform component-specific initialization logic for init().
     *
     * \return True if successful.
     */
    virtual bool initImpl() = 0;
    /*!
     * \brief Perform component-specific closing logic for close().
     *
     * \return True if successful.
     */
    virtual bool closeImpl() = 0;
    //
    virtual void loadRuntimeConfImpl(boost::property_tree::ptree&& pConf);  ///< Perform component-specific loading of runtime configuration.
    virtual boost::property_tree::ptree dumpRuntimeConfImpl() const;        ///< Perform component-specific saving of runtime configuration.

protected:
    const Layer layer;                              ///< %Layer that this layer component belongs to.
    const std::string type;                         ///< Name given to this type (i.e. specific derived class) of layer component.
    const std::string name;                         ///< Name given to the current component instance.
    //
    const LayerConfig config;                       ///< Component-specific configuration tree (typically from loaded YAML file).
    //
    bool initialized;                               ///< Initialized and not closed.
    //
    const ContextualLogger logger;                  ///< Logger instance (automatically adding information about this component to each entry).

private:
    const std::string selfDescription;              ///< Standard description of the layer component for logging purposes.

public:
    /*!
     * \brief Enumeration of identifiers for the distinguished component layers
     *        \ref Layers::TL "TL", \ref Layers::HL "HL" and \ref Layers::RL "RL".
     */
    enum class Layer : std::uint8_t
    {
        TransferLayer = 0,  ///< Transfer layer (TL): Interfaces
        HardwareLayer = 1,  ///< Hardware layer (HL): Drivers
        RegisterLayer = 2   ///< Register layer (RL): Registers
    };
};

/*!
 * \brief Starting point for the differentiation into the three layers of the basil layer structure with their associated layer components.
 *
 * For every of those layers there is a dedicated nested namespace, in which LayerBase is specialized according to the peculiarities of the
 * respective layer and in which the actual layer component classes are implemented (which in turn derive from those initial specializations):
 * - The \ref LayerBase::Layer::TransferLayer "TransferLayer" is formed by the \ref Layers::TL "TL" namespace.
 * - The \ref LayerBase::Layer::HardwareLayer "HardwareLayer" is formed by the \ref Layers::HL "HL" namespace.
 * - The \ref LayerBase::Layer::RegisterLayer "RegisterLayer" is formed by the \ref Layers::RL "RL" namespace.
 */
namespace Layers {}

} // namespace casil

#endif // CASIL_LAYERBASE_H
