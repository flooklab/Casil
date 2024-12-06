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

#ifndef CASIL_TL_SERIAL_H
#define CASIL_TL_SERIAL_H

#include <casil/TL/directinterface.h>

#include <casil/layerconfig.h>
#include <casil/layerfactorymacros.h>
#include <casil/TL/CommonImpl/serialportwrapper.h>

#include <cstdint>
#include <string>
#include <vector>

namespace casil
{

namespace TL
{

/*!
 * \brief %Interface for communication via a serial port.
 *
 * \todo Detailed doc
 */
class Serial final : public DirectInterface
{
public:
    Serial(std::string pName, LayerConfig pConfig);     ///< Constructor.
    ~Serial() override = default;                       ///< Default destructor.
    //
    std::vector<std::uint8_t> read(int pSize = -1) override;
    void write(const std::vector<std::uint8_t>& pData) override;
    std::vector<std::uint8_t> query(const std::vector<std::uint8_t>& pData, int pSize = -1) override;
    //
    bool readBufferEmpty() const override;
    void clearReadBuffer() override;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    const std::string port;                 ///< Serial port identifier (e.g. device file).
    const std::string readTermination;      ///< Read termination to detect end of read data stream.
    const std::string writeTermination;     ///< Write termination to append to written data.
    const int baudRate;                     ///< Baud rate setting for the serial communication.
    //
    CommonImpl::SerialPortWrapper serialPortWrapper;    ///< Detailed serial port logic wrapper.

    CASIL_REGISTER_INTERFACE_H("Serial")
};

} // namespace TL

} // namespace casil

#endif // CASIL_TL_SERIAL_H
