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

#ifndef CASIL_LAYERBASE_H
#define CASIL_LAYERBASE_H

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>

namespace casil
{

/*!
 * \brief Common top-level base class that connects the different layers and layer components of the basil layer structure.
 *
 * \todo Detailed doc
 */
class LayerBase
{
public:
    enum class Layer : std::uint8_t;

public:
    LayerBase(Layer pLayer, std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
                                                    ///< Constructor.
    LayerBase(const LayerBase&) = delete;           ///< Deleted copy constructor.
    LayerBase(LayerBase&&) = default;               ///< Default move constructor.
    virtual ~LayerBase() = default;                 ///< Default destructor.
    //
    LayerBase& operator=(LayerBase) = delete;       ///< Deleted copy assignment operator.
    LayerBase& operator=(LayerBase&&) = delete;     ///< Deleted move assignment operator.
    //
    Layer getLayer() const;                         ///< Get the layer of this layer component.
    const std::string& getType() const;             ///< Get the type name of this layer component.
    const std::string& getName() const;             ///< Get the instance name of this layer component.
    //
    bool init(bool pForce = false);                 ///< Initialize this layer component.
    bool close(bool pForce = false);                ///< Close ("uninitialize") this layer component.

protected:
    const std::string& getSelfDescription() const;  ///< Get a standard description of this layer component for logging purposes.

private:
    virtual bool initImpl() = 0;                    ///< Perform component-specific initialization logic for init().
    virtual bool closeImpl() = 0;                   ///< Perform component-specific closing logic for close().

protected:
    const Layer layer;                              ///< %Layer that this layer component belongs to.
    const std::string type;                         ///< Name given to this type (i.e. specific derived class) of layer component.
    const std::string name;                         ///< Name given to the current component instance.
    //
    const LayerConfig config;                       ///< Component-specific configuration tree (typically from loaded YAML file).
    //
    bool initialized;                               ///< Initialized and not closed.

private:
    const std::string selfDescription;              ///< Standard description of the layer component for logging purposes.

public:
    /*!
     * \brief Enumeration of identifiers for the distinguished component layers TL, HL and RL.
     *
     * \todo Detailed doc
     */
    enum class Layer : std::uint8_t
    {
        TransferLayer = 0,  ///< Transfer layer (TL): Interfaces
        HardwareLayer = 1,  ///< Hardware layer (HL): Drivers
        RegisterLayer = 2   ///< Register layer (RL): Registers
    };
};

} // namespace casil

#endif // CASIL_LAYERBASE_H
