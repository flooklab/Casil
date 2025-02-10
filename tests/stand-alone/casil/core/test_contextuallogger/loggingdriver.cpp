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

#include "loggingdriver.h"

#include <casil/logger.h>

#include <utility>

using casil::HL::LoggingDriver;

CASIL_REGISTER_DRIVER_CPP(LoggingDriver)

//

LoggingDriver::LoggingDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Private

bool LoggingDriver::initImpl()
{
    logger.logCritical("This is not a regular log but just a test message! (1)");
    logger.logError("This is not a regular log but just a test message! (2)");
    logger.logWarning("This is not a regular log but just a test message! (3)");
    logger.logSuccess("This is not a regular log but just a test message! (4)");
    logger.logInfo("This is not a regular log but just a test message! (5)");
    logger.logMore("This is not a regular log but just a test message! (6)");
    logger.logVerbose("This is not a regular log but just a test message! (7)");
    logger.logDebug("This is not a regular log but just a test message! (8)");
    logger.logDebugDebug("This is not a regular log but just a test message! (9)");
    logger.log("This is not a regular log but just a test message! (10)", Logger::LogLevel::Info);
    logger.log("This is not a regular log but just a test message! (11)", Logger::LogLevel::None);
    return true;
}

bool LoggingDriver::closeImpl()
{
    logger.log("This is not an error but yet another a test message! (0)", Logger::LogLevel::Error);
    logger.logError("This is not an error but yet another a test message! (1)");
    logger.logCritical("This is not an error but yet another a test message! (2)");
    return true;
}
