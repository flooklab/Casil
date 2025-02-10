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

#ifndef CASIL_CONTEXTUALLOGGER_H
#define CASIL_CONTEXTUALLOGGER_H

#include <casil/logger.h>

#include <string>

namespace casil
{

class LayerBase;

/*!
 * \brief Print log messages with contextual information.
 *
 * Uses Logger to provide logging identical to Logger (see there) except that
 * contextual information is automatically prepended to the passed log messages.
 */
class ContextualLogger
{
public:
    using LogLevel = Logger::LogLevel;  ///< \copybrief Logger::LogLevel

public:
    explicit ContextualLogger(const LayerBase& pComponent);                         ///< Constructor for logging from layer components.
    //
    void log(const std::string& pMessage, LogLevel pLevel = LogLevel::Info) const;  ///< Print a log message with contextual information.
    //
    void logCritical(const std::string& pMessage) const;    ///< Print a log message with contextual information (LogLevel::Critical).
    void logError(const std::string& pMessage) const;       ///< Print a log message with contextual information (LogLevel::Error).
    void logWarning(const std::string& pMessage) const;     ///< Print a log message with contextual information (LogLevel::Warning).
    void logSuccess(const std::string& pMessage) const;     ///< Print a log message with contextual information (LogLevel::Success).
    void logInfo(const std::string& pMessage) const;        ///< Print a log message with contextual information (LogLevel::Info).
    void logMore(const std::string& pMessage) const;        ///< Print a log message with contextual information (LogLevel::More).
    void logVerbose(const std::string& pMessage) const;     ///< Print a log message with contextual information (LogLevel::Verbose).
    void logDebug(const std::string& pMessage) const;       ///< Print a log message with contextual information (LogLevel::Debug).
    void logDebugDebug(const std::string& pMessage) const;  ///< Print a log message with contextual information (LogLevel::DebugDebug).

private:
    const std::string contextPrefix;                        ///< Prefix for every log message that describes the contextual information.
};

} // namespace casil

#endif // CASIL_CONTEXTUALLOGGER_H
