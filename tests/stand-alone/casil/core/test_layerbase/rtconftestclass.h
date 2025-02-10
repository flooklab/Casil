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

#ifndef CASILTESTS_RTCONFTESTCLASS_H
#define CASILTESTS_RTCONFTESTCLASS_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

#include <boost/property_tree/ptree_fwd.hpp>

class RTConfTestClass final : public casil::LayerBase
{
public:
    RTConfTestClass(const casil::LayerConfig& pConfig, const casil::LayerConfig& pRequiredConfig);
    ~RTConfTestClass() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;
    //
    bool loadRuntimeConfImpl(boost::property_tree::ptree&& pConf) override;
    boost::property_tree::ptree dumpRuntimeConfImpl() const override;

private:
    int someNumber;
};

#endif // CASILTESTS_RTCONFTESTCLASS_H
