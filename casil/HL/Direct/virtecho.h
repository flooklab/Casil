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

#ifndef CASIL_LAYERS_HL_VIRTECHO_H
#define CASIL_LAYERS_HL_VIRTECHO_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <string>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief Pseudo driver to write back to its interface what can be read from it.
 *
 * Reads some data \e on \e request and immediately writes it back (see operator()()).
 */
class VirtEcho final : public DirectDriver
{
public:
    VirtEcho(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);    ///< Constructor.
    ~VirtEcho() override = default;                                                     ///< Default destructor.
    //
    void operator()(int pN) const;                                                      ///< Read and immediately write back a number of bytes.

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_DRIVER_H("VirtEcho")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_VIRTECHO_H
