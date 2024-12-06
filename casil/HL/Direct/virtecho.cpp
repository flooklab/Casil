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

#include <casil/HL/Direct/virtecho.h>

#include <utility>

using casil::HL::VirtEcho;

CASIL_REGISTER_DRIVER_CPP(VirtEcho)

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
VirtEcho::VirtEcho(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \brief Read and immediately write back a number of bytes.
 *
 * \todo Detailed doc
 *
 * \param pN
 */
void VirtEcho::operator()(const int pN) const
{
    interface.write(interface.read(pN));
}

//Private

/*!
 * \copybrief DirectDriver::initImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool VirtEcho::initImpl()
{
    return true;
}

/*!
 * \copybrief DirectDriver::closeImpl()
 *
 * \todo Detailed doc
 *
 * \return
 */
bool VirtEcho::closeImpl()
{
    return true;
}
