/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 M. Frohne
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

#include <casil/contextuallogger.h>
#include <casil/layerbase.h>
#include <casil/logger.h>

using casil::ContextualLogger;

void bind_ContextualLogger(py::module& pM)
{
    py::class_<ContextualLogger>(pM, "ContextualLogger", "Print log messages with contextual information.")
            .def(py::init<const casil::LayerBase&>(), "Constructor for logging from layer components.", py::arg("component"))
            .def("log", &ContextualLogger::log, "Print a log message with contextual information.",
                 py::arg("message"), py::arg("level") = casil::Logger::LogLevel::Info)
            .def("logCritical", &ContextualLogger::logCritical, "Print a log message with contextual information (LogLevel.Critical).",
                 py::arg("message"))
            .def("logError", &ContextualLogger::logError, "Print a log message with contextual information (LogLevel.Error).",
                 py::arg("message"))
            .def("logWarning", &ContextualLogger::logWarning, "Print a log message with contextual information (LogLevel.Warning).",
                 py::arg("message"))
            .def("logSuccess", &ContextualLogger::logSuccess, "Print a log message with contextual information (LogLevel.Success).",
                 py::arg("message"))
            .def("logInfo", &ContextualLogger::logInfo, "Print a log message with contextual information (LogLevel.Info).",
                 py::arg("message"))
            .def("logMore", &ContextualLogger::logMore, "Print a log message with contextual information (LogLevel.More).",
                 py::arg("message"))
            .def("logVerbose", &ContextualLogger::logVerbose, "Print a log message with contextual information (LogLevel.Verbose).",
                 py::arg("message"))
            .def("logDebug", &ContextualLogger::logDebug, "Print a log message with contextual information (LogLevel.Debug).",
                 py::arg("message"))
            .def("logDebugDebug", &ContextualLogger::logDebugDebug, "Print a log message with contextual information (LogLevel.DebugDebug).",
                 py::arg("message"));
}
