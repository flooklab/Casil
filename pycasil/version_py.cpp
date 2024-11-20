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

#include <casil/version.h>

namespace Version = casil::Version;

struct PyCasilVersion {};   //Helper struct to work around missing module-level (read-only) properties

void bindVersion(py::module& pM)
{
    using Version::ReleaseType;

    py::enum_<ReleaseType>(pM, "ReleaseType", "")
            .value("Alpha", ReleaseType::Alpha, "")
            .value("Beta", ReleaseType::Beta, "")
            .value("ReleaseCandidate", ReleaseType::ReleaseCandidate, "")
            .value("Normal", ReleaseType::Normal, "");

    py::class_<PyCasilVersion>(pM, "CasilVersion", "")
            .def_readonly_static("casilVersionMajor", &Version::casilVersionMajor, "")
            .def_readonly_static("casilVersionMinor", &Version::casilVersionMinor, "")
            .def_readonly_static("casilVersionPatch", &Version::casilVersionPatch, "")
            .def_readonly_static("casilVersionType", &Version::casilVersionType, "");

    pM.def("toString", &Version::toString, "");
}
