/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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

#ifndef CASILTESTS_LOGGINGDRIVER_H
#define CASILTESTS_LOGGINGDRIVER_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <string>

namespace casil
{

namespace Layers::HL
{

class LoggingDriver final : public DirectDriver
{
public:
    LoggingDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);
    ~LoggingDriver() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;

    CASIL_REGISTER_DRIVER_H("Logging-Driver")
};

} // namespace HL

} // namespace casil

#endif // CASILTESTS_LOGGINGDRIVER_H
