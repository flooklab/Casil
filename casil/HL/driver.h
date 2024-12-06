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

#ifndef CASIL_HL_DRIVER_H
#define CASIL_HL_DRIVER_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

/*!
 * \brief Hardware layer: Drivers that control the connected devices/components.
 *
 * \todo Detailed doc
 */
namespace HL
{

/*!
 * \brief Common base class for all driver components in the hardware layer (HL).
 *
 * \todo Detailed doc
 */
class Driver : public LayerBase
{
public:
    Driver(std::string pType, std::string pName, LayerConfig pConfig, const LayerConfig& pRequiredConfig);  ///< Constructor.
    ~Driver() override = default;                                                                           ///< Default destructor.
    //
    virtual void reset();                                                                           ///< Reset the controlled device/module.
    //
    /*!
     * \brief Get driver-specific special data.
     *
     * \param pSize Potentially number of bytes to get (implementation-defined).
     * \param pAddrOffs Potentially data offset as number of bytes (implementation-defined).
     * \return Requested data as byte sequence.
     */
    virtual std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) = 0;
    /*!
     * \brief Set driver-specific special data.
     *
     * \param pData Data to be set as byte sequence.
     * \param pAddrOffs Potentially data offset as number of bytes (implementation-defined).
     */
    virtual void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) = 0;
    virtual void exec() = 0;                                                                        ///< Perform a driver-specific action.
    /*!
     * \brief Check if a driver-specific action has finished.
     *
     * \return True if finished.
     */
    virtual bool isDone() = 0;
    //
    //TODO could something be added here to enable direct C++ FunctionalRegister-functionality without python wrapper in between?

private:
    bool initImpl() override = 0;       ///< Perform driver-specific initialization logic for init().
    bool closeImpl() override = 0;      ///< Perform driver-specific closing logic for init().
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_DRIVER_H
