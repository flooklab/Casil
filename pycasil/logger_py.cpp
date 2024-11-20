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

#include <casil/logger.h>

using casil::Logger;

void bind_Logger(py::module& pM)
{
    py::class_<Logger> logger(pM, "Logger", "");

    py::enum_<Logger::LogLevel>(logger, "LogLevel", "")
            .value("None", Logger::LogLevel::None, "")
            .value("Critical", Logger::LogLevel::Critical, "")
            .value("Error", Logger::LogLevel::Error, "")
            .value("Warning", Logger::LogLevel::Warning, "")
            .value("Success", Logger::LogLevel::Success, "")
            .value("Info", Logger::LogLevel::Info, "")
            .value("More", Logger::LogLevel::More, "")
            .value("Verbose", Logger::LogLevel::Verbose, "")
            .value("Debug", Logger::LogLevel::Debug, "")
            .value("DebugDebug", Logger::LogLevel::DebugDebug, "");

    logger.def_static("getLogLevel", &Logger::getLogLevel, "")
            .def_static("setLogLevel", &Logger::setLogLevel, "", py::arg("level"))
            .def_static("addOutputCout", &Logger::addOutputCout, "")
            .def_static("removeOutputCout", &Logger::removeOutputCout, "")
            .def_static("addOutputCerr", &Logger::addOutputCerr, "")
            .def_static("removeOutputCerr", &Logger::removeOutputCerr, "")
            .def_static("addOutputClog", &Logger::addOutputClog, "")
            .def_static("removeOutputClog", &Logger::removeOutputClog, "")
            .def_static("addLogFile", &Logger::addLogFile, "", py::arg("fileName"))
            .def_static("removeLogFile", &Logger::removeLogFile, "", py::arg("fileName"))
            .def_static("log", &Logger::log, "", py::arg("message"), py::arg("level") = Logger::LogLevel::Info)
            .def_static("logCritical", &Logger::logCritical, "", py::arg("message"))
            .def_static("logError", &Logger::logError, "", py::arg("message"))
            .def_static("logWarning", &Logger::logWarning, "", py::arg("message"))
            .def_static("logSuccess", &Logger::logSuccess, "", py::arg("message"))
            .def_static("logInfo", &Logger::logInfo, "", py::arg("message"))
            .def_static("logMore", &Logger::logMore, "", py::arg("message"))
            .def_static("logVerbose", &Logger::logVerbose, "", py::arg("message"))
            .def_static("logDebug", &Logger::logDebug, "", py::arg("message"))
            .def_static("logDebugDebug", &Logger::logDebugDebug, "", py::arg("message"));
}
