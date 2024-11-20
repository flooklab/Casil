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

#ifndef CASIL_LOGGER_H
#define CASIL_LOGGER_H

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <string_view>

namespace casil
{

/*!
 * \brief Print log messages.
 *
 * Log messages can be written to arbitrary instances of \e std::ostream. These can be
 * added/removed via addOutput() / removeOutput(). For the standard streams there are the
 * shortcuts addOutputCout(), addOutputCerr(), addOutputClog() and corresponding removers.
 *
 * Log files can be either added as ostreams (see above) or automatically opened and added
 * via the file name using addLogFile(), respectively removed and closed using removeLogFile().
 *
 * Logging a message can be done by calling log(). See there for details about the formatting etc.
 *
 * The Logger's current log level (see Logger::LogLevel) defines which messages will be printed by
 * comparing it to each message's own log level. The Logger's log level can be set via setLogLevel().
 *
 * Note that in addition to log() there is also a shortcut function for every log level:
 * logCritical(), ..., logDebugDebug().
 *
 * Note that the writing of each log message is protected by a mutex, such that logging also works from multiple threads.
 * See also log().
 */
class Logger
{
public:
    /*!
     * \brief Enumeration of available log levels.
     *
     * Only log messages that have a log level below or equal to the currently set log level.
     */
    enum class LogLevel : std::uint8_t
    {
        None       =  0,    ///< Do not print any log messages.
        Critical   = 10,    ///< Log critical errors only.
        Error      = 20,    ///< Log all errors.
        Warning    = 30,    ///< Log also warnings.
        Success    = 40,    ///< Log also success messages.
        Info       = 50,    ///< Log also normal notifications.
        More       = 60,    ///< Log also less important notifications.
        Verbose    = 70,    ///< Log even more notifications.
        Debug      = 80,    ///< Log also debug messages.
        DebugDebug = 90     ///< Log even more debug messages.
    };

public:
    Logger() = delete;                                                                  ///< Deleted constructor.
    //
    static LogLevel getLogLevel();                                                      ///< Get the log level.
    static void setLogLevel(LogLevel pLevel);                                           ///< Set the log level.
    //
    static void addOutput(std::ostream& pStream);                                       ///< Add a log output stream.
    static void removeOutput(const std::ostream& pStream);                              ///< Remove a log output stream.
    //
    static void addOutputCout();                                                        ///< Add std::cout to log output streams.
    static void removeOutputCout();                                                     ///< Remove std::cout from log output streams.
    static void addOutputCerr();                                                        ///< Add std::cerr to log output streams.
    static void removeOutputCerr();                                                     ///< Remove std::cerr from log output streams.
    static void addOutputClog();                                                        ///< Add std::clog to log output streams.
    static void removeOutputClog();                                                     ///< Remove std::clog from log output streams.
    //
    static bool addLogFile(const std::string& pFileName);                               ///< Add a log file to log output streams.
    static void removeLogFile(const std::string& pFileName);                            ///< Remove a log file from log output streams.
    //
    static void log(std::string_view pMessage, LogLevel pLevel = LogLevel::Info);       ///< Print a log message.
    //
    static void logCritical(std::string_view pMessage);                                 ///< Print a log message (LogLevel::Critical).
    static void logError(std::string_view pMessage);                                    ///< Print a log message (LogLevel::Error).
    static void logWarning(std::string_view pMessage);                                  ///< Print a log message (LogLevel::Warning).
    static void logSuccess(std::string_view pMessage);                                  ///< Print a log message (LogLevel::Success).
    static void logInfo(std::string_view pMessage);                                     ///< Print a log message (LogLevel::Info).
    static void logMore(std::string_view pMessage);                                     ///< Print a log message (LogLevel::More).
    static void logVerbose(std::string_view pMessage);                                  ///< Print a log message (LogLevel::Verbose).
    static void logDebug(std::string_view pMessage);                                    ///< Print a log message (LogLevel::Debug).
    static void logDebugDebug(std::string_view pMessage);                               ///< Print a log message (LogLevel::DebugDebug).

private:
    static void logMessage(std::string_view pMessage, LogLevel pLevel);                 ///< Format and print a log message.
    //
    static std::string logLevelToLabel(LogLevel pLevel);                                ///< Get the label for a log level.
    static LogLevel labelToLogLevel(const std::string& pLevel);                         ///< Get the log level from its label.

private:
    static LogLevel logLevel;                                                           ///< Defines, which log messages are accepted.
    //
    static std::list<std::reference_wrapper<std::ostream>> outputStreams;               ///< List of used output streams.
    static std::map<std::string, std::ofstream> files;                                  ///< Map of open log files.
    //
    static std::mutex logMutex;                                                         ///< Mutex to allow logging from different threads.
};

} // namespace casil

#endif // CASIL_LOGGER_H
