/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2026 M. Frohne
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

#include <pycasil/pycasil.h>

extern void bindHL_Driver(py::module&);

extern void bindHL_DirectDriver(py::module&);
extern void bindHL_MuxedDriver(py::module&);

extern void bindHL_MetaDriver(py::module&);
extern void bindHL_RegisterDriver(py::module&);

extern void bindHL_DummyDriver(py::module&);
extern void bindHL_SCPI(py::module&);
extern void bindHL_VirtEcho(py::module&);

extern void bindHL_DummyMetaDriver(py::module&);
extern void bindHL_DummyMuxedDriver(py::module&);
extern void bindHL_DummyRegisterDriver(py::module&);
extern void bindHL_GPIO(py::module&);
extern void bindHL_I2C(py::module&);
extern void bindHL_Si570(py::module&);
extern void bindHL_SiTCPFifo(py::module&);
extern void bindHL_SPI(py::module&);

void bindHL(py::module& pM)
{
    bindHL_Driver(pM);

    bindHL_DirectDriver(pM);
    bindHL_MuxedDriver(pM);

    bindHL_MetaDriver(pM);
    bindHL_RegisterDriver(pM);

    bindHL_DummyDriver(pM);
    bindHL_SCPI(pM);
    bindHL_VirtEcho(pM);

    bindHL_DummyMetaDriver(pM);
    bindHL_DummyMuxedDriver(pM);
    bindHL_DummyRegisterDriver(pM);
    bindHL_GPIO(pM);
    bindHL_I2C(pM);
    bindHL_Si570(pM);
    bindHL_SiTCPFifo(pM);
    bindHL_SPI(pM);
}
