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

#ifndef CASILTESTS_RTCONFDRIVER_H
#define CASILTESTS_RTCONFDRIVER_H

#include <casil/HL/directdriver.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <string>

namespace casil
{

namespace Layers::HL
{

class RTConfDriver final : public DirectDriver
{
public:
    RTConfDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig);
    ~RTConfDriver() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;
    //
    bool loadRuntimeConfImpl(boost::property_tree::ptree&& pConf) override;
    boost::property_tree::ptree dumpRuntimeConfImpl() const override;

private:
    int someNumber;

    CASIL_REGISTER_DRIVER_H("RTConfDrv")
};

} // namespace HL

} // namespace casil

#endif // CASILTESTS_RTCONFDRIVER_H
