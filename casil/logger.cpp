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

#include <casil/logger.h>

#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>

using casil::Logger;

Logger::LogLevel Logger::logLevel = Logger::LogLevel::None;
//
std::list<std::reference_wrapper<std::ostream>> Logger::outputStreams = {};
std::map<std::string, std::ofstream> Logger::files = {};
//
std::mutex Logger::logMutex;

//Public

/*!
 * \brief Get the log level.
 *
 * \return Current log level.
 */
Logger::LogLevel Logger::getLogLevel()
{
    return logLevel;
}

/*!
 * \brief Set the log level.
 *
 * \param pLevel New log level.
 */
void Logger::setLogLevel(const LogLevel pLevel)
{
    logLevel = pLevel;
}

//

/*!
 * \brief Add a log output stream.
 *
 * Adds \p pStream to the list of active log output sinks.
 *
 * \param pStream Output stream to be added.
 */
void Logger::addOutput(std::ostream& pStream)
{
    std::lock_guard<std::mutex> logLock(logMutex);
    (void)logLock;

    outputStreams.push_front(std::ref(pStream));
}

/*!
 * \brief Remove a log output stream.
 *
 * Removes \p pStream from the list of active log output sinks.
 *
 * \param pStream Output stream to be removed.
 */
void Logger::removeOutput(const std::ostream& pStream)
{
    std::lock_guard<std::mutex> logLock(logMutex);
    (void)logLock;

    for (decltype(outputStreams)::const_iterator it = outputStreams.begin(); it != outputStreams.end(); ++it)
    {
        const std::ostream& tStream = it->get();
        if (&tStream == &pStream)
        {
            outputStreams.erase(it);
            return;
        }
    }
}

//

/*!
 * \brief Add std::cout to log output streams.
 *
 * See addOutput() with std::cout as stream argument.
 */
void Logger::addOutputCout()
{
    addOutput(std::cout);
}

/*!
 * \brief Remove std::cout from log output streams.
 *
 * See removeOutput() with std::cout as stream argument.
 */
void Logger::removeOutputCout()
{
    removeOutput(std::cout);
}

/*!
 * \brief Add std::cerr to log output streams.
 *
 * See addOutput() with std::cerr as stream argument.
 */
void Logger::addOutputCerr()
{
    addOutput(std::cerr);
}

/*!
 * \brief Remove std::cerr from log output streams.
 *
 * See removeOutput() with std::cerr as stream argument.
 */
void Logger::removeOutputCerr()
{
    removeOutput(std::cerr);
}

/*!
 * \brief Add std::clog to log output streams.
 *
 * See addOutput() with std::clog as stream argument.
 */
void Logger::addOutputClog()
{
    addOutput(std::clog);
}

/*!
 * \brief Remove std::clog from log output streams.
 *
 * See removeOutput() with std::clog as stream argument.
 */
void Logger::removeOutputClog()
{
    removeOutput(std::clog);
}

//

/*!
 * \brief Add a log file to log output streams.
 *
 * Opens a file with file name \p pFileName and adds it as log output stream (see addOutput()).
 *
 * \param pFileName File name of the log file.
 * \return If successful.
 */
bool Logger::addLogFile(const std::string& pFileName)
{
    if (files.contains(pFileName))
        return false;

    try
    {
        std::ofstream logFile;
        logFile.exceptions(std::ios_base::badbit | std::ios_base::failbit);
        logFile.open(pFileName);

        files.insert({pFileName, std::move(logFile)});

        addOutput(files[pFileName]);
    }
    catch (const std::ios_base::failure&)
    {
        std::cerr<<"ERROR: Could not open log file \"" + pFileName + "\"!"<<std::endl;
        return false;
    }

    return true;
}

/*!
 * \brief Remove a log file from log output streams.
 *
 * Removes the previously added log file \p pFileName from the log output streams (see removeOutput()) and closes it.
 *
 * \param pFileName File name of the log file.
 */
void Logger::removeLogFile(const std::string& pFileName)
{
    const auto it = files.find(pFileName);

    if (it == files.end())
        return;

    std::ofstream& ofstrm = it->second;

    removeOutput(ofstrm);

    try
    {
        ofstrm.flush();
        ofstrm.close();
    }
    catch (const std::exception&)
    {
        std::cerr<<"ERROR: Could not flush/close log file \"" + pFileName + "\"!"<<std::endl;
    }

    files.erase(pFileName);
}

//

/*!
 * \brief Print a log message.
 *
 * Logs the message \p pMessage, if \p pLevel is lower than or equal to the Logger's current log level.
 * Messages with \p pLevel LogLevel::None will never be printed.
 *
 * The actual logging is done through logMessage(), which adds timestamps and
 * allows for safe logging from different threads. See there for more details.
 *
 * \param pMessage The message to log.
 * \param pLevel The log level of \p pMessage.
 */
void Logger::log(const std::string_view pMessage, const LogLevel pLevel)
{
    if (pLevel == LogLevel::None)
        return;

    if (static_cast<std::uint8_t>(pLevel) > static_cast<std::uint8_t>(logLevel))
        return;

    logMessage(pMessage, pLevel);
}

//

/*!
 * \brief Print a log message (LogLevel::Critical).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Critical.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logCritical(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Critical);
}

/*!
 * \brief Print a log message (LogLevel::Error).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Error.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logError(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Error);
}

/*!
 * \brief Print a log message (LogLevel::Warning).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Warning.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logWarning(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Warning);
}

/*!
 * \brief Print a log message (LogLevel::Success).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Success.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logSuccess(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Success);
}

/*!
 * \brief Print a log message (LogLevel::Info).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Info.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logInfo(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Info);
}

/*!
 * \brief Print a log message (LogLevel::More).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::More.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logMore(const std::string_view pMessage)
{
    log(pMessage, LogLevel::More);
}

/*!
 * \brief Print a log message (LogLevel::Verbose).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Verbose.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logVerbose(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Verbose);
}

/*!
 * \brief Print a log message (LogLevel::Debug).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::Debug.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logDebug(const std::string_view pMessage)
{
    log(pMessage, LogLevel::Debug);
}

/*!
 * \brief Print a log message (LogLevel::DebugDebug).
 *
 * Logs the message \p pMessage, if the Logger's current log level is higher than or equal to LogLevel::DebugDebug.
 *
 * See also log().
 *
 * \param pMessage The message to log.
 */
void Logger::logDebugDebug(const std::string_view pMessage)
{
    log(pMessage, LogLevel::DebugDebug);
}

//Private

/*!
 * \brief Format and print a log message.
 *
 * Logs a formatted log message with text \p pMessage ("MESSAGE") prepended
 * by the current timestamp, \p pLevel ("LEVEL") and the thread ID ("xxxx"):
 *
 * "[YYYY-MM-DDThh:mm:ssGMT, LEVEL|xxxx] MESSAGE"
 *
 * The log output is written to all previously added output streams (including log files).
 * See also addOutput() and addLogFile().
 *
 * The logging is protected by a mutex, i.e. log messages from different threads will be displayed correctly.
 *
 * \param pMessage The message to log.
 * \param pLevel The log level of \p pMessage.
 */
void Logger::logMessage(const std::string_view pMessage, const LogLevel pLevel)
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::ostringstream osstr;
    osstr<<"["<<std::put_time(std::gmtime(&time), "%FT%T%Z")<<", "<<std::setw(5)<<logLevelToLabel(pLevel)<<std::setw(0)
         <<"|"<<std::this_thread::get_id()<<"] "<<pMessage<<"\n";

    std::string msg = osstr.str();

    //Output log message to all configured output streams
    {
        std::lock_guard<std::mutex> logLock(logMutex);
        (void)logLock;

        for (std::ostream& ostr : outputStreams)
            ostr<<msg;

        //Flush immediately for warnings and more severe messages
        if (static_cast<std::uint8_t>(pLevel) <= static_cast<std::uint8_t>(LogLevel::Warning))
        {
            for (std::ostream& ostr : outputStreams)
            {
                try
                {
                    ostr.flush();
                }
                catch (const std::ios_base::failure&)
                {
                    std::cerr<<"ERROR: Could not flush log output stream!"<<std::endl;
                }
            }
        }
    }
}

//

/*!
 * \brief Get the label for a log level.
 *
 * Returns a unique label for log level \p pLevel. Converting back is possible using labelToLogLevel().
 *
 * \param pLevel The log level to get a label for.
 * \return The corresponding label for \p pLevel (max. 5 characters long).
 */
std::string Logger::logLevelToLabel(const LogLevel pLevel)
{
    switch (pLevel)
    {
        case LogLevel::None:
            return "NONE";
        case LogLevel::Critical:
            return "CRIT";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Warning:
            return "WARNG";
        case LogLevel::Success:
            return "SUCC";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::More:
            return "MORE";
        case LogLevel::Verbose:
            return "VERB";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::DebugDebug:
            return "DDBUG";
        default:
            return "NONE";
    }
}

/*!
 * \brief Get the log level from its label.
 *
 * Get the log level corresponding to a label \p pLevel that was previously obtained via logLevelToLabel().
 *
 * \param pLevel The label representing the requested log level.
 * \return The log level corresponding to label \p pLevel.
 */
Logger::LogLevel Logger::labelToLogLevel(const std::string& pLevel)
{
    if (pLevel == "NONE")
        return LogLevel::None;
    else if (pLevel == "CRIT")
        return LogLevel::Critical;
    else if (pLevel == "ERROR")
        return LogLevel::Error;
    else if (pLevel == "WARNG")
        return LogLevel::Warning;
    else if (pLevel == "SUCC")
        return LogLevel::Success;
    else if (pLevel == "INFO")
        return LogLevel::Info;
    else if (pLevel == "MORE")
        return LogLevel::More;
    else if (pLevel == "VERB")
        return LogLevel::Verbose;
    else if (pLevel == "DEBUG")
        return LogLevel::Debug;
    else if (pLevel == "DDBUG")
        return LogLevel::DebugDebug;
    else
        return LogLevel::None;
}
