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

#include <casil/HL/Direct/dummydriver.h>

#include <casil/logger.h>

#include <utility>

using casil::HL::DummyDriver;

CASIL_REGISTER_DRIVER_CPP(DummyDriver)

//

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pName
 * \param pInterface
 * \param pConfig
 */
DummyDriver::DummyDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Private

/*!
 * \copybrief DirectDriver::initImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DummyDriver::initImpl()
{
    Logger::logDebug("Called initImpl() for " + getSelfDescription() + ".");
    return true;
}

/*!
 * \copybrief DirectDriver::closeImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DummyDriver::closeImpl()
{
    Logger::logDebug("Called closeImpl() for " + getSelfDescription() + ".");
    return true;
}
