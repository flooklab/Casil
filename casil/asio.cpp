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

#include <casil/asio.h>

#include <casil/logger.h>

#include <boost/asio/executor_work_guard.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <system_error>

namespace
{

using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

/*
 * Creates a static "work guard" for the IO context object (from getIOContext()) and always returns that one,
 * except if reset() was called on the work guard before; then a new work guard instance will be assigned.
 *
 * This guard is used to keep the threads from startRunIOContext() running all the time until explicitly stopped by stopRunIOContext().
 */
WorkGuard& getWorkGuard()
{
    static std::unique_ptr<WorkGuard> work;

    //Create a new work guard if it was not yet created or if it was reset before
    if (!work || !(work->owns_work()))
    {
        if (work)
            work->reset();

        work.reset();
        work = std::make_unique<WorkGuard>(boost::asio::make_work_guard(casil::ASIO::getIOContext()));
    }

    return *work;
}

} // namespace

using casil::ASIO;

bool ASIO::ioContextRunning = false;
std::vector<std::thread> ASIO::ioContextThreads;

//Public

/*!
 * \brief Get the IO context object.
 *
 * Creates a static IO context object and always returns that one.
 *
 * See also the \e Boost documentation for \c boost::asio::io_context.
 *
 * \return The IO context object.
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
 * Starts \p pNumThreads processing threads that are responsible for executing async IO handlers for respective
 * async requests made to the Boost %ASIO library. Sets up a "work guard" to keep the threads running even if
 * no handlers are scheduled at some time. Hence stopping the threads is achieved with stopRunIOContext().
 *
 * See also Auxil::AsyncIORunner for a RAII approach of running these threads.
 *
 * If IO context threads are already/still running (see ioContextThreadsRunning()), this function will do nothing but return false.
 *
 * \param pNumThreads Number of threads to start.
 * \return True if the threads were started.
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

    ::getWorkGuard();   //Set up "work guard"

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
 * Resets the work guard set up by and joins the threads started by startRunIOContext().
 */
void ASIO::stopRunIOContext()
{
    Logger::logInfo("Stopping all IO context threads...");

    ::getWorkGuard().reset();

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
 * Checks whether threads were started via startRunIOContext() and not stopped via stopRunIOContext() yet.
 *
 * \return True if threads are running.
 */
bool ASIO::ioContextThreadsRunning()
{
    return ioContextRunning;
}
