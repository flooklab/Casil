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

#ifndef CASIL_HL_MUXEDDRIVER_H
#define CASIL_HL_MUXEDDRIVER_H

#include <casil/HL/driver.h>

#include <casil/layerconfig.h>
#include <casil/TL/muxedinterface.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace HL
{

/*!
 * \brief Base class to derive from for driver components that control firmware modules on a basil bus connected via a MuxedInterface.
 *
 * \todo Detailed doc
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
    std::vector<std::uint8_t> getData(int pSize = -1, std::uint32_t pAddrOffs = 0) override;        ///< Set driver-specific special data.
    void setData(const std::vector<std::uint8_t>& pData, std::uint32_t pAddrOffs = 0) override;     ///< Get driver-specific special data.
    void exec() override;                                                                           ///< Perform a driver-specific action.
    bool isDone() override;                                                                 ///< Check if a driver-specific action has finished.

protected:
    std::vector<std::uint8_t> read(std::uint64_t pAddr, int pSize = -1) const;      ///< Read from the interface relative to the base address.
    void write(std::uint64_t pAddr, const std::vector<std::uint8_t>& pData) const;  ///< Write to the interface relative to the base address.
    std::vector<std::uint8_t> query(std::uint64_t pWriteAddr, std::uint64_t pReadAddr,
                                    const std::vector<std::uint8_t>& pData, int pSize = -1) const;  ///< \brief Write a query to the interface
                                                                                                    ///  and read the response, both relative
                                                                                                    ///  to the base address.

private:
    bool initImpl() override = 0;
    bool closeImpl() override = 0;

protected:
    InterfaceBaseType& interface;           ///< The interface instance to be used for required access to the transfer layer.
    //
    const std::uint64_t baseAddr;           ///< The root bus address for the controlled firmware module instance.
};

} // namespace HL

} // namespace casil

#endif // CASIL_HL_MUXEDDRIVER_H
