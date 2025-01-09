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

#include <casil/logger.h>

using casil::Logger;

void bind_Logger(py::module& pM)
{
    py::class_<Logger> logger(pM, "Logger", "Print log messages.");

    py::enum_<Logger::LogLevel>(logger, "LogLevel", "Enumeration of available log levels.")
            .value("None", Logger::LogLevel::None, "Do not print any log messages.")
            .value("Critical", Logger::LogLevel::Critical, "Log critical errors only.")
            .value("Error", Logger::LogLevel::Error, "Log all errors.")
            .value("Warning", Logger::LogLevel::Warning, "Log also warnings.")
            .value("Success", Logger::LogLevel::Success, "Log also success messages.")
            .value("Info", Logger::LogLevel::Info, "Log also normal notifications.")
            .value("More", Logger::LogLevel::More, "Log also less important notifications.")
            .value("Verbose", Logger::LogLevel::Verbose, "Log even more notifications.")
            .value("Debug", Logger::LogLevel::Debug, "Log also debug messages.")
            .value("DebugDebug", Logger::LogLevel::DebugDebug, "Log even more debug messages.");

    logger.def_static("getLogLevel", &Logger::getLogLevel, "Get the log level.")
            .def_static("setLogLevel", &Logger::setLogLevel, "Set the log level.", py::arg("level"))
            .def_static("addOutputCout", &Logger::addOutputCout, "Add stdout to log output streams.")
            .def_static("removeOutputCout", &Logger::removeOutputCout, "Remove stdout from log output streams.")
            .def_static("addOutputCerr", &Logger::addOutputCerr, "Add stderr to log output streams.")
            .def_static("removeOutputCerr", &Logger::removeOutputCerr, "Remove stderr from log output streams.")
            .def_static("addOutputClog", &Logger::addOutputClog, "Add \"stdlog\" (see C++'s std::clog) to log output streams.")
            .def_static("removeOutputClog", &Logger::removeOutputClog, "Remove \"stdlog\" (see C++'s std::clog) from log output streams.")
            .def_static("addLogFile", &Logger::addLogFile, "Add a log file to log output streams.", py::arg("fileName"))
            .def_static("removeLogFile", &Logger::removeLogFile, "Remove a log file from log output streams.", py::arg("fileName"))
            .def_static("log", &Logger::log, "Print a log message.", py::arg("message"), py::arg("level") = Logger::LogLevel::Info)
            .def_static("logCritical", &Logger::logCritical, "Print a log message (LogLevel.Critical).", py::arg("message"))
            .def_static("logError", &Logger::logError, "Print a log message (LogLevel.Error).", py::arg("message"))
            .def_static("logWarning", &Logger::logWarning, "Print a log message (LogLevel.Warning).", py::arg("message"))
            .def_static("logSuccess", &Logger::logSuccess, "Print a log message (LogLevel.Success).", py::arg("message"))
            .def_static("logInfo", &Logger::logInfo, "Print a log message (LogLevel.Info).", py::arg("message"))
            .def_static("logMore", &Logger::logMore, "Print a log message (LogLevel.More).", py::arg("message"))
            .def_static("logVerbose", &Logger::logVerbose, "Print a log message (LogLevel.Verbose).", py::arg("message"))
            .def_static("logDebug", &Logger::logDebug, "Print a log message (LogLevel.Debug).", py::arg("message"))
            .def_static("logDebugDebug", &Logger::logDebugDebug, "Print a log message (LogLevel.DebugDebug).", py::arg("message"));
}
