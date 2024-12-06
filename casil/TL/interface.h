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

#ifndef CASIL_TL_INTERFACE_H
#define CASIL_TL_INTERFACE_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <chrono>
#include <string>

namespace casil
{

/*!
 * \brief Transfer layer: Interfaces that connect the Casil host to its devices/components.
 *
 * \todo Detailed doc
 */
namespace TL
{

/*!
 * \brief Common base class for all interface components in the transfer layer (TL).
 *
 * \todo Detailed doc
 */
class Interface : public LayerBase
{
public:
    Interface(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);   ///< Constructor.
    ~Interface() override = default;                                                                            ///< Default destructor.
    //
    virtual bool readBufferEmpty() const = 0;   ///< Check if the read buffer is empty.
    virtual void clearReadBuffer() = 0;         ///< Clear the current contents of the read buffer.

private:
    bool initImpl() override = 0;               ///< Perform interface-specific initialization logic for init().
    bool closeImpl() override = 0;              ///< Perform interface-specific closing logic for init().

protected:
    const double queryDelay;                    ///< Configured delay value for query operations (between write and read) in milliseconds.
    const std::chrono::microseconds queryDelayMicroSecs;    ///< Rounded chrono version of queryDelay.
};

} // namespace TL

} // namespace casil

#endif // CASIL_TL_INTERFACE_H
