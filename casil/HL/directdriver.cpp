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

#include <casil/HL/directdriver.h>

#include <utility>

using casil::HL::DirectDriver;

/*!
 * \brief Constructor.
 *
 * \todo Detailed doc
 *
 * \param pType
 * \param pName
 * \param pInterface
 * \param pConfig
 * \param pRequiredConfig
 */
DirectDriver::DirectDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface,
                           LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    Driver(std::move(pType), std::move(pName), std::move(pConfig), pRequiredConfig),
    interface(pInterface)
{
}

//Private

/*!
 * \brief Override not intended to be used (dummy implementation).
 *
 * \todo Detailed doc
 *
 * \return
 */
std::vector<std::uint8_t> DirectDriver::getData(int, std::uint32_t)
{
    return {};  //(sic!)
}

/*!
 * \brief Override not intended to be used (dummy implementation).
 *
 * \todo Detailed doc
 */
void DirectDriver::setData(const std::vector<std::uint8_t>&, std::uint32_t)
{
    //(sic!)
}

/*!
 * \brief Override not intended to be used (dummy implementation).
 *
 * \todo Detailed doc
 */
void DirectDriver::exec()
{
    //(sic!)
}

/*!
 * \brief Override not intended to be used (dummy implementation).
 *
 * \todo Detailed doc
 *
 * \return
 */
bool DirectDriver::isDone()
{
    return false;   //(sic!)
}
