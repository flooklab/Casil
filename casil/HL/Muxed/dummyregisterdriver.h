/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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

#ifndef CASIL_LAYERS_HL_DUMMYREGISTERDRIVER_H
#define CASIL_LAYERS_HL_DUMMYREGISTERDRIVER_H

#include <casil/HL/registerdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <cstdint>
#include <string>

namespace casil
{

namespace Layers::HL
{

/*!
 * \brief Dummy implementation of RegisterDriver without actual functionality.
 *
 * Available registers:
 *
 * <table>
 * <tr><th>Name</th><th>\ref casil::HL::RegisterDescr::DataType "DataType"</th><th>\ref casil::HL::RegisterDescr::AccessMode "AccessMode"</th>
 * <th>Size</th><th>Description</th></tr>
 * <tr><td>\c RESET</td>    <td>Value</td>    <td>WriteOnly</td><td>8</td>          <td>Soft reset active on write to address.</td></tr>
 * <tr><td>\c VERSION</td>  <td>Value</td>    <td>ReadOnly</td> <td>8</td>          <td>Firmware module version.</td></tr>
 * </table>
 *
 * Note: It makes no sense to use these registers in this very dummy implementation as no dummy firmware module
 * counterpart is provided, but they are still defined here for convenience/consistency because every firmware
 * module on the basil bus is supposed to implement this basic reset and version functionality anyway.
 */
class DummyRegisterDriver final : public RegisterDriver
{
public:
    DummyRegisterDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);     ///< Constructor.
    ~DummyRegisterDriver() override = default;                                                      ///< Default destructor.

private:
    bool initModule() override;
    //
    void resetImpl() override;
    //
    std::uint8_t getModuleSoftwareVersion() const override;
    std::uint8_t getModuleFirmwareVersion() override;

private:
    static constexpr std::uint8_t requireFirmwareVersion = 0;   ///< Compatible version of the controlled firmware module.

    CASIL_REGISTER_DRIVER_H("DummyRegisterDriver")
};

} // namespace Layers::HL

} // namespace casil

#endif // CASIL_LAYERS_HL_DUMMYREGISTERDRIVER_H
