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

#include <pycasil/pycasil.h>

#include <casil/layerbase.h>

using casil::LayerBase;

void bind_LayerBase(py::module& pM)
{
    py::class_<LayerBase> layerBase(pM, "LayerBase", "Common top-level base class that connects the different layers "
                                                     "and layer components of the basil layer structure.");

    py::native_enum<LayerBase::Layer>(layerBase, "Layer", "enum.Enum",
                                      "Enumeration of identifiers for the distinguished component layers TL, HL and RL.")
            .value("TransferLayer", LayerBase::Layer::TransferLayer, "Transfer layer (TL): Interfaces")
            .value("HardwareLayer", LayerBase::Layer::HardwareLayer, "Hardware layer (HL): Drivers")
            .value("RegisterLayer", LayerBase::Layer::RegisterLayer, "Register layer (RL): Registers")
            .finalize();

    layerBase.def("getLayer", &LayerBase::getLayer, "Get the layer of this layer component.")
            .def("getType", &LayerBase::getType, "Get the type name of this layer component.")
            .def("getName", &LayerBase::getName, "Get the instance name of this layer component.")
            .def("init", &LayerBase::init, "Initialize this layer component.", py::arg("force") = false)
            .def("close", &LayerBase::close, "Close (\"uninitialize\") this layer component.", py::arg("force") = false)
            .def("loadRuntimeConfiguration", &LayerBase::loadRuntimeConfiguration,
                 "Load additional, component-specific configuration data/values.", py::arg("conf"))
            .def("dumpRuntimeConfiguration", &LayerBase::dumpRuntimeConfiguration,
                 "Save current state of component-specific configuration data/values.");
}
