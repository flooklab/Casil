/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024–2025 M. Frohne
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

extern void bind_ASIO(py::module&);
extern void bind_Device(py::module&);
extern void bind_LayerBase(py::module&);
extern void bind_LayerConfig(py::module&);
extern void bind_Logger(py::module&);

extern void bindAuxil(py::module&);
extern void bindBytes(py::module&);
extern void bindEnv(py::module&);
extern void bindVersion(py::module&);

extern void bindHL(py::module&);
extern void bindRL(py::module&);
extern void bindTL(py::module&);

PYBIND11_MODULE(PyCasil, pyCasil)
{
    pyCasil.doc() = "Python binding of Casil, a reimplementation of the data acquisition framework basil in C++.";

    bind_ASIO(pyCasil);
    bind_Device(pyCasil);
    bind_LayerBase(pyCasil);
    bind_LayerConfig(pyCasil);
    bind_Logger(pyCasil);

    //

    py::module modAuxil = pyCasil.def_submodule("Auxil", "Auxiliary functions and classes.");
    bindAuxil(modAuxil);

    py::module modBytes = pyCasil.def_submodule("Bytes", "Auxiliary functions for bit/byte manipulation.");
    bindBytes(modBytes);

    py::module modEnv = pyCasil.def_submodule("Env", "Handling of environment variables needed by the library.");
    bindEnv(modEnv);

    py::module modVersion = pyCasil.def_submodule("Version", "Library version information.");
    bindVersion(modVersion);

    //

    py::module modLayers = pyCasil.def_submodule("Layers", "Starting point for the differentiation into the three layers of "
                                                           "the basil layer structure with their associated layer components.");

    py::module modTL = modLayers.def_submodule("TL", "Transfer layer: Interfaces that connect the PyCasil host to its devices/components.");
    bindTL(modTL);

    py::module modHL = modLayers.def_submodule("HL", "Hardware layer: Drivers that control the connected devices/components.");
    bindHL(modHL);

    py::module modRL = modLayers.def_submodule("RL", "Register layer: Abstraction for register(-like) functionalities of the drivers.");
    bindRL(modRL);
}
