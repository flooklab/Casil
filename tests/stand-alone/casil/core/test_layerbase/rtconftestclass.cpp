/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2025 M. Frohne
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

#include "rtconftestclass.h"

#include <boost/property_tree/ptree.hpp>

#include <stdexcept>
#include <string>

//

RTConfTestClass::RTConfTestClass(const casil::LayerConfig& pConfig, const casil::LayerConfig& pRequiredConfig) :
    LayerBase(casil::LayerBase::Layer::TransferLayer, "RTConfTestClass", "", pConfig, pRequiredConfig),
    someNumber(5)
{
}

//Private

bool RTConfTestClass::initImpl()
{
    someNumber = 2;
    return true;
}

bool RTConfTestClass::closeImpl()
{
    return true;
}

//

bool RTConfTestClass::loadRuntimeConfImpl(boost::property_tree::ptree&& pConf)
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

boost::property_tree::ptree RTConfTestClass::dumpRuntimeConfImpl() const
{
    if (someNumber == 59)
        throw std::runtime_error("Let's deliberately fail...");

    using boost::property_tree::ptree;
    ptree tree;
    ptree subTree;
    subTree.data() = std::to_string(someNumber);
    tree.push_back({"some_number", subTree});
    return tree;
}
