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

#include <casil/contextuallogger.h>

#include <casil/layerbase.h>

using casil::ContextualLogger;

/*!
 * \brief Constructor for logging from layer components.
 *
 * Configures the contextual information to be prepended to every passed log message as
 * 'LAYER/TYPE/"NAME": ', with \c LAYER one of \e TL, \e HL or \e RL according to LayerBase::getLayer(),
 * \c TYPE equal to LayerBase::getType() and \c NAME equal to LayerBase::getName(), each taken from \p pComponent.
 *
 * \note Will \e only call the functions LayerBase::getLayer(), LayerBase::getType() and LayerBase::getName() on \p pComponent.
 *
 * \param pComponent The layer component to provide contextual logging information for.
 */
ContextualLogger::ContextualLogger(const LayerBase& pComponent) :
    contextPrefix(
        (pComponent.getLayer() == LayerBase::Layer::TransferLayer ? "TL" :
                                                                    (pComponent.getLayer() == LayerBase::Layer::HardwareLayer ? "HL" :
                                                                                                                                "RL")) +
        std::string("/") + pComponent.getType() + "/\"" + pComponent.getName() + "\": "
        )
{
}

//Public

/*!
 * \brief Print a log message with contextual information.
 *
 * See Logger::log(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * \param pMessage The message to log.
 * \param pLevel The log level of \p pMessage.
 */
void ContextualLogger::log(const std::string& pMessage, const LogLevel pLevel) const
{
    //Use duplicated check as shortcut before concatenating the strings below
    if (!Logger::includeLogLevel(pLevel))
        return;

    Logger::log(contextPrefix + pMessage, pLevel);
}

//

/*!
 * \brief Print a log message with contextual information (LogLevel::Critical).
 *
 * See Logger::logCritical(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logCritical(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Critical);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Error).
 *
 * See Logger::logError(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logError(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Error);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Warning).
 *
 * See Logger::logWarning(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logWarning(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Warning);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Success).
 *
 * See Logger::logSuccess(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logSuccess(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Success);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Info).
 *
 * See Logger::logInfo(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logInfo(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Info);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::More).
 *
 * See Logger::logMore(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logMore(const std::string& pMessage) const
{
    log(pMessage, LogLevel::More);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Verbose).
 *
 * See Logger::logVerbose(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logVerbose(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Verbose);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::Debug).
 *
 * See Logger::logDebug(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logDebug(const std::string& pMessage) const
{
    log(pMessage, LogLevel::Debug);
}

/*!
 * \brief Print a log message with contextual information (LogLevel::DebugDebug).
 *
 * See Logger::logDebugDebug(), except that contextual information from ContextualLogger() is prepended to \p pMessage.
 *
 * Note: Forwards to log().
 *
 * \param pMessage The message to log.
 */
void ContextualLogger::logDebugDebug(const std::string& pMessage) const
{
    log(pMessage, LogLevel::DebugDebug);
}
