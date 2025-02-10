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

#include <casil/version.h>

namespace Version = casil::Version;

struct PyCasilVersion {};   //Helper struct to work around missing module-level (read-only) properties

void bindVersion(py::module& pM)
{
    using Version::ReleaseType;

    py::enum_<ReleaseType>(pM, "ReleaseType", "Release type of the released library version.")
            .value("Alpha", ReleaseType::Alpha, "Pre-release alpha status.")
            .value("Beta", ReleaseType::Beta, "Pre-release beta status.")
            .value("ReleaseCandidate", ReleaseType::ReleaseCandidate, "Pre-release release candidate status.")
            .value("Normal", ReleaseType::Normal, "Normal/final release.");

    py::class_<PyCasilVersion>(pM, "CasilVersion", "Current library version.")
            .def_readonly_static("casilVersionMajor", &Version::casilVersionMajor, "Library version major number.")
            .def_readonly_static("casilVersionMinor", &Version::casilVersionMinor, "Library version minor number.")
            .def_readonly_static("casilVersionPatch", &Version::casilVersionPatch, "Library version patch number.")
            .def_readonly_static("casilVersionType", &Version::casilVersionType, "Library version release type.");

    pM.def("toString", &Version::toString, "Get the library version formatted as string.");
}
