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
 * \brief Common top-level base class that connects the different layers and components of the basil layer structure.
 *
 * \todo Detailed doc
 */
class LayerBase
{
public:
    enum class Layer : std::uint8_t;

public:
    LayerBase(Layer pLayer, std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
    LayerBase(const LayerBase&) = delete;
    LayerBase(LayerBase&&) = default;
    virtual ~LayerBase() = default;
    //
    LayerBase& operator=(LayerBase) = delete;
    LayerBase& operator=(LayerBase&&) = delete;
    //
    Layer getLayer() const;
    const std::string& getType() const;
    const std::string& getName() const;
    //
    bool init(bool pForce = false);
    bool close(bool pForce = false);

protected:
    const std::string& getSelfDescription() const;

private:
    virtual bool initImpl() = 0;
    virtual bool closeImpl() = 0;

protected:
    const Layer layer;
    const std::string type;
    const std::string name;
    //
    const LayerConfig config;
    //
    bool initialized;

private:
    const std::string selfDescription;

public:
    /*!
     * \brief Enumeration of identifiers for the distinguished component layers TL, HL and RL.
     *
     * \todo Detailed doc
     */
    enum class Layer : std::uint8_t
    {
        TransferLayer = 0,
        HardwareLayer = 1,
        RegisterLayer = 2
    };
};

} // namespace casil

#endif // CASIL_LAYERBASE_H
