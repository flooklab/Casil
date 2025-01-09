/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

#include <casil/RL/dummyregister.h>

#include <utility>

using casil::Layers::RL::DummyRegister;

CASIL_REGISTER_REGISTER_CPP(DummyRegister)

//

/*!
 * \brief Constructor.
 *
 * \param pName Component instance name.
 * \param pDriver %Driver instance to be used.
 * \param pConfig Component configuration.
 */
DummyRegister::DummyRegister(std::string pName, HL::Driver& pDriver, LayerConfig pConfig) :
    Register(typeName, std::move(pName), pDriver, std::move(pConfig), LayerConfig())
{
}

//Private

/*!
 * \copybrief Register::initImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool DummyRegister::initImpl()
{
    return true;
}

/*!
 * \copybrief Register::closeImpl()
 *
 * Does nothing.
 *
 * \return True.
 */
bool DummyRegister::closeImpl()
{
    return true;
}
