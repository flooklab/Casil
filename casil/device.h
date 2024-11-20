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

#ifndef CASIL_DEVICE_H
#define CASIL_DEVICE_H

#include <casil/layerbase.h>
#include <casil/HL/driver.h>
#include <casil/RL/register.h>
#include <casil/TL/interface.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace casil
{

class Device
{
private:
    Device();

public:
    explicit Device(const boost::property_tree::ptree& pConfig);
    explicit Device(const std::string& pConfig);
    Device(const Device&) = delete;
    Device(Device&&) = default;
    virtual ~Device();
    //
    Device& operator=(Device) = delete;
    Device& operator=(Device&&) = delete;
    //
    LayerBase& operator[](std::string_view pName) const;
    //
    TL::Interface& interface(std::string_view pName) const;
    HL::Driver& driver(std::string_view pName) const;
    RL::Register& reg(std::string_view pName) const;
    //
    bool init(bool pForce = false);
    bool close(bool pForce = false);

private:
    std::map<std::string, const std::unique_ptr<TL::Interface>, std::less<>> interfaces;
    std::map<std::string, const std::unique_ptr<HL::Driver>, std::less<>> drivers;
    std::map<std::string, const std::unique_ptr<RL::Register>, std::less<>> registers;
    //
    bool initialized;
};

} // namespace casil

#endif // CASIL_DEVICE_H
