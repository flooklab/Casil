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

#include <pycasil/pycasil.h>

extern void bindTL_Interface(py::module&);

extern void bindTL_DirectInterface(py::module&);
extern void bindTL_MuxedInterface(py::module&);

extern void bindTL_DummyInterface(py::module&);
extern void bindTL_Serial(py::module&);
extern void bindTL_TCP(py::module&);
extern void bindTL_UDP(py::module&);

extern void bindTL_DummyMuxedInterface(py::module&);
extern void bindTL_SiTCP(py::module&);

void bindTL(py::module& pM)
{
    bindTL_Interface(pM);

    bindTL_DirectInterface(pM);
    bindTL_MuxedInterface(pM);

    bindTL_DummyInterface(pM);
    bindTL_Serial(pM);
    bindTL_TCP(pM);
    bindTL_UDP(pM);

    bindTL_DummyMuxedInterface(pM);
    bindTL_SiTCP(pM);
}
