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

#include "rtconfdriver.h"

#include <boost/property_tree/ptree.hpp>

#include <utility>

using casil::HL::RTConfDriver;

CASIL_REGISTER_DRIVER_CPP(RTConfDriver)

//

RTConfDriver::RTConfDriver(std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig) :
    DirectDriver(typeName, std::move(pName), pInterface, std::move(pConfig), LayerConfig()),
    someNumber(123)
{
}

//Private

bool RTConfDriver::initImpl()
{
    return true;
}

bool RTConfDriver::closeImpl()
{
    return true;
}

//

bool RTConfDriver::loadRuntimeConfImpl(boost::property_tree::ptree&& pConf)
{
    try
    {
        someNumber = pConf.get_child("some_number").get_value<int>();
        return true;
    }
    catch (const boost::property_tree::ptree_error&)
    {
        return false;
    }
}

boost::property_tree::ptree RTConfDriver::dumpRuntimeConfImpl() const
{
    using boost::property_tree::ptree;
    ptree tree;
    ptree subTree;
    subTree.data() = std::to_string(someNumber);
    tree.push_back({"some_number", subTree});
    return tree;
}
