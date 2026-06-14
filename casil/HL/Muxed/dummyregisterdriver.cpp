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

#include <casil/HL/Muxed/dummyregisterdriver.h>

#include <utility>

using casil::Layers::HL::DummyRegisterDriver;

CASIL_REGISTER_DRIVER_CPP(DummyRegisterDriver)

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
DummyRegisterDriver::DummyRegisterDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    RegisterDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig(),
       {{"RESET",     {.type{DT::Value},     .mode{AM::WriteOnly}, .addr{0},  .size{8},  .offs{0}}},
        {"VERSION",   {.type{DT::Value},     .mode{AM::ReadOnly},  .addr{0},  .size{8},  .offs{0}}}})
{
}

//Private

/*!
 * \copybrief RegisterDriver::initModule()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \return True.
 */
bool DummyRegisterDriver::initModule()
{
    logger.logDebug("initModule() was called.");
    return true;
}

//

/*!
 * \copybrief RegisterDriver::resetImpl()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 */
void DummyRegisterDriver::resetImpl()
{
    logger.logDebug("resetImpl() was called.");
}

//

/*!
 * \copybrief RegisterDriver::getModuleSoftwareVersion()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * \copydetails RegisterDriver::getModuleSoftwareVersion()
 */
std::uint8_t DummyRegisterDriver::getModuleSoftwareVersion() const
{
    logger.logDebug("getModuleSoftwareVersion() was called.");
    return requireFirmwareVersion;
}

/*!
 * \copybrief RegisterDriver::getModuleFirmwareVersion()
 *
 * Does nothing except \ref casil::Logger::LogLevel::Debug "Debug"-logging the function call.
 *
 * The returned "firmware" version is faked as zero.
 *
 * \copydetails RegisterDriver::getModuleFirmwareVersion()
 */
std::uint8_t DummyRegisterDriver::getModuleFirmwareVersion()
{
    logger.logDebug("getModuleFirmwareVersion() was called.");
    return 0;
}
