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

#include <casil/asio.h>

using casil::ASIO;

void bind_ASIO(py::module& pM)
{
    py::class_<ASIO>(pM, "ASIO", "Limited interface to the used async IO back end from the Boost library.")
            .def_static("startRunIOContext", &ASIO::startRunIOContext, "Start threads that continuously execute/run the IO context.",
                        py::arg("numThreads") = 1)
            .def_static("stopRunIOContext", &ASIO::stopRunIOContext, "Stop all running IO context threads.")
            .def_static("ioContextThreadsRunning", &ASIO::ioContextThreadsRunning, "Check if any IO context threads are currently running.");
}
