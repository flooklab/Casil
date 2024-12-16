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
 * \param pName Component instance name.
 * \param pInterface %Interface instance to be used.
 * \param pConfig Component configuration.
 */
VirtEcho::VirtEcho(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig())
{
}

//Public

/*!
 * \brief Read and immediately write back a number of bytes.
 *
 * Reads via TL::DirectInterface::read() and then writes via TL::DirectInterface::write().
 *
 * \param pN Number of bytes as taken by TL::DirectInterface::read() and TL::DirectInterface::write().
 */
void VirtEcho::operator()(const int pN) const
{
    interface.write(interface.read(pN));
}

//Private

/*!
 * \copybrief DirectDriver::initImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool VirtEcho::initImpl()
{
    return true;
}

/*!
 * \copybrief DirectDriver::closeImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool VirtEcho::closeImpl()
{
    return true;
}
