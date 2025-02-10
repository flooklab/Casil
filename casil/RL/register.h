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

#ifndef CASIL_LAYERS_RL_REGISTER_H
#define CASIL_LAYERS_RL_REGISTER_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>
#include <casil/HL/driver.h>

#include <string>

namespace casil
{

namespace Layers
{

/*!
 * \brief %Register layer: Abstraction for register(-like) functionalities of the drivers.
 *
 * This is the third layer with the most high-level abstraction, which provides advanced possibilities to access hardware
 * registers of firmware modules, to use special driver functionality as if it was a simple register or to simplify using
 * driver functionality in other ways (e.g. automatically binding certain function arguments such as device channel numbers).
 *
 * \todo The last aspect would be a "FunctionalRegister" as in basil; not implemented and not sure if this will
 *       ever be implemented \e directly in the C++ library in a satisfying way (maybe just too pythonic)
 *       --> i.e. need to perhaps revisit this description at some point
 *
 * To accomplish that a Register uses the HL::Driver API from the hardware layer (HL). Note, though,
 * that the interchangeability of the driver components to be used for the register components is \e less
 * than between drivers (HL) and interfaces (TL), simply because of the higher specialization/abstraction
 * of the hardware layer with respect to the transfer layer. Using an arbitrary driver for a specific
 * register does not \e necessarily crash but often just does not make any sense.
 *
 * Note that a different way of register access is also available \e directly in the hardware layer by using "register drivers"
 * (see RegisterDriver). The register layer is not a substitute for that and should rather be used in a complementary way.
 */
namespace RL
{

/*!
 * \brief Common base class for all register components in the register layer (RL).
 *
 * This class provides very basic/limited functionality likely common to all registers.
 * Use this class as base class for implementing any register.
 */
class Register : public LayerBase
{
public:
    Register(std::string pType, std::string pName, HL::Driver& pDriver, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
                                        ///< Constructor.
    ~Register() override = default;     ///< Default destructor.

private:
    /*!
     * \brief Perform register-specific initialization logic for init().
     *
     * \copydetails LayerBase::initImpl()
     */
    bool initImpl() override = 0;
    /*!
     * \brief Perform register-specific closing logic for init().
     *
     * \copydetails LayerBase::closeImpl()
     */
    bool closeImpl() override = 0;

protected:
    HL::Driver& driver;                 ///< The driver instance to be used for required access to the hardware layer.
};

} // namespace RL

} // namespace Layers

namespace RL = Layers::RL;

} // namespace casil

#endif // CASIL_LAYERS_RL_REGISTER_H
