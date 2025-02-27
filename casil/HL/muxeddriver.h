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

#ifndef CASIL_LAYERS_HL_MUXEDDRIVER_H
#define CASIL_LAYERS_HL_MUXEDDRIVER_H

#include <casil/HL/driver.h>

#include <casil/layerconfig.h>
#include <casil/TL/muxedinterface.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace Layers
{

namespace HL
{

/*!
 * \brief Base class to derive from for driver components that control firmware modules on a basil bus connected via a TL::MuxedInterface.
 *
 * Use this class as base class for implementing a "muxed" driver, i.e. a driver which controls one
 * (dependent) \e addressable \e part of a hardware device, such as a firmware module running on FPGA
 * hardware based on the basil bus. In turn, this hardware must be connected to via a "muxed" interface.
 *
 * Consider to instead use the further specialized RegisterDriver as base class for muxed
 * drivers that can/must mainly control the firmware module using register operations.
 *
 * Note: This class is in contrast to DirectDriver, which should be used for
 * independent/stand-alone hardware that is connected to via a "direct" interface.
 */
class MuxedDriver : public Driver
{
public:
    typedef TL::MuxedInterface InterfaceBaseType;   ///< Alias for proper \ref TL::Interface "Interface" sub-type (analog to DirectDriver).

public:
    MuxedDriver(std::string pType, std::string pName, InterfaceBaseType& pInterface, LayerConfig pConfig, const LayerConfig& pRequiredConfig);
                                            ///< Constructor.
    ~MuxedDriver() override = default;      ///< Default destructor.
    //
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override;
    void exec() override;
    bool isDone() override;

protected:
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) const;      ///< Read from the interface relative to the base address.
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) const;  ///< Write to the interface relative to the base address.
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) const;  ///< \brief Write a query to the interface
                                                                                                    ///  and read the response, both relative
                                                                                                    ///  to the base address.

private:
    /*!
     * \brief Initialize the firmware module.
     *
     * \copydetails Driver::initImpl()
     */
    bool initImpl() override = 0;
    /*!
     * \brief Close the firmware module.
     *
     * \copydetails Driver::closeImpl()
     */
    bool closeImpl() override = 0;

protected:
    InterfaceBaseType& interface;           ///< The interface instance to be used for required access to the transfer layer.
    //
    const std::uint64_t baseAddr;           ///< The root bus address for the controlled firmware module instance.
};

} // namespace HL

} // namespace Layers

} // namespace casil

#endif // CASIL_LAYERS_HL_MUXEDDRIVER_H
