/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 M. Frohne
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

#include <casil/HL/metadriver.h>

#include <boost/property_tree/ptree.hpp>

#include <stdexcept>
#include <utility>

namespace
{

using casil::LayerConfig;
using boost::property_tree::ptree;

/*
 * Returns a new configuration copied from 'pConfig' but with the child node "base_addr" with value "0x0" added at the top level.
 * Throws std::runtime_error if 'pConfig' already contains a "base_addr" node.
 */
LayerConfig insertDummyBaseAddr(const LayerConfig& pConfig)
{
    if (pConfig.contains(LayerConfig::fromYAML("{base_addr: }"), false))
        throw std::runtime_error("Base address (\"base_addr\") must not be set for meta drivers.");

    ptree tree = pConfig.getRawTreeAt("");

    ptree subTree;
    subTree.put_value("0x0");
    tree.add_child("base_addr", subTree);

    return LayerConfig(tree);
}

} // namespace

using casil::Layers::HL::MetaDriver;

//

/*!
 * \brief Constructor.
 *
 * Constructs MuxedDriver by forwarding \p pType, \p pName, the interface of \p pBackendDriver, \p pConfig and \p pRequiredConfig.
 * Note that a "base_addr" node with value "0x0" is added to \p pConfig in order to comply with the MuxedDriver requirements.
 * The "base_addr" value should not be defined in the passed \p pConfig already, otherwise an exception is thrown.
 *
 * Initializes the internal muxed driver reference (for use as the actual/backend driver) from \p pBackendDriver.
 *
 * \throws std::runtime_error If \p pConfig contains a "base_addr" value.
 *
 * \param pType Registered component type name.
 * \param pName Component instance name.
 * \param pBackendDriver %Driver instance to be used as actual/backend driver.
 * \param pConfig Component configuration.
 * \param pRequiredConfig Configuration required to be specified by \p pConfig.
 */
MetaDriver::MetaDriver(std::string pType, std::string pName, MuxedDriver& pBackendDriver,
                       const LayerConfig pConfig, const LayerConfig& pRequiredConfig) :
    MuxedDriver(std::move(pType), std::move(pName), MetaDriverFriendAccessHelper::getDriverInterface(pBackendDriver),
                ::insertDummyBaseAddr(pConfig), pRequiredConfig),
    backendDriver(pBackendDriver)
{
}
