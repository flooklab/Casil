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

#include <casil/asio.h>

#include <casil/logger.h>

#include <memory>
#include <sstream>
#include <string>
#include <system_error>

using casil::ASIO;

bool ASIO::ioContextRunning = false;
std::vector<std::thread> ASIO::ioContextThreads;

//Public

/*!
 * \brief Get the IO context object.
 *
 * \todo Detailed doc
 *
 * \return
 */
boost::asio::io_context& ASIO::getIOContext()
{
    static boost::asio::io_context ioContext;
    return ioContext;
}

//

/*!
 * \brief Start threads that continuously execute/run the IO context.
 *
 * \todo Detailed doc
 *
 * \param pNumThreads
 * \return
 */
bool ASIO::startRunIOContext(const unsigned int pNumThreads)
{
    if (ioContextRunning)
        return false;

    if (pNumThreads == 0)
        return false;

    Logger::logInfo("Starting " + std::to_string(pNumThreads) + " IO context threads...");

    boost::asio::io_context& ioContextRef = getIOContext();
    ioContextRef.restart();

    getWorkGuard(); //Set up "work guard"

    for (unsigned int i = 0; i < pNumThreads; ++i)
    {
        try
        {
            boost::asio::io_context *const ioContextPtr = &ioContextRef;    //Need to be pedantic and capture pointer by value
            ioContextThreads.emplace_back(
                        [ioContextPtr]()
                        {
                            std::ostringstream threadIdStrm;
                            threadIdStrm<<std::this_thread::get_id();

                            Logger::logDebug("Started IO context thread " + threadIdStrm.str() + ".");

                            ioContextPtr->run();

                            Logger::logDebug("Finished IO context thread " + threadIdStrm.str() + ".");
                        });
        }
        catch (const std::system_error& exc)
        {
            Logger::logError(std::string("Exception while starting IO context threads: ") + exc.what());
            Logger::logWarning("Stopping already started threads...");

            stopRunIOContext();

            return false;
        }
    }

    ioContextRunning = true;

    Logger::logSuccess("Started " + std::to_string(pNumThreads) + " IO context threads.");

    return true;
}

/*!
 * \brief Stop all running IO context threads.
 *
 * \todo Detailed doc
 */
void ASIO::stopRunIOContext()
{
    Logger::logInfo("Stopping all IO context threads...");

    getWorkGuard().reset();

    for (auto& thread : ioContextThreads)
    {
        try
        {
            thread.join();
        }
        catch (const std::system_error& exc)
        {
            Logger::logWarning(std::string("Could not join an IO context thread: ") + exc.what());
        }
    }

    ioContextThreads.clear();

    ioContextRunning = false;

    Logger::logSuccess("Stopped all IO context threads.");
}

//

/*!
 * \brief Check if any IO context threads are currently running.
 *
 * \todo Detailed doc
 *
 * \return
 */
bool ASIO::ioContextThreadsRunning()
{
    return ioContextRunning;
}

//Private

/*!
 * \brief Get the "work guard" object for the IO context object.
 *
 * \todo Detailed doc
 *
 * \return
 */
ASIO::WorkGuard& ASIO::getWorkGuard()
{
    static std::unique_ptr<WorkGuard> work;

    //Create a new work guard if it was not yet created or if it was reset before
    if (!work || !(work->owns_work()))
    {
        if (work)
            work->reset();

        work.reset();
        work = std::make_unique<WorkGuard>(boost::asio::make_work_guard(getIOContext()));
    }

    return *work;
}
