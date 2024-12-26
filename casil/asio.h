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

#ifndef CASIL_ASIO_H
#define CASIL_ASIO_H

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include <thread>
#include <vector>

namespace casil
{

/*!
 * \brief Limited interface to the used async IO back end from the Boost library.
 *
 * Some parts of the Casil library (see TL, \ref Layers::TL::Interface "TL::Interface") use asynchronous IO functionality
 * from Boost %ASIO. This class provides access to the required, limited subset of Boost %ASIO,
 * which basically consists of an "IO context" object (see getIOContext()) and running processing
 * threads (see startRunIOContext()) to execute handlers for asynchronous requests.
 *
 * At least one processing thread must be running for some of the \ref Layers::TL "TL" interfaces to function properly.
 * In these cases (or simply always) start the thread(s) \e before calling Device::init()
 * (or \ref Layers::TL::Interface::init() "TL::Interface::init()").
 */
class ASIO
{
public:
    ASIO() = delete;                                                ///< Deleted constructor.
    //
    static boost::asio::io_context& getIOContext();                 ///< Get the IO context object.
    //
    static bool startRunIOContext(unsigned int pNumThreads = 1);    ///< Start threads that continuously execute/run the IO context.
    static void stopRunIOContext();                                 ///< Stop all running IO context threads.
    //
    static bool ioContextThreadsRunning();                          ///< Check if any IO context threads are currently running.

private:
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
                                                                    ///< "Work guard" for IO context threads (see the \e Boost documentation).
    static WorkGuard& getWorkGuard();                               ///< Get the "work guard" object for the IO context object.

private:
    static bool ioContextRunning;                                   ///< Flags whether IO context threads were started and not stopped yet.
    static std::vector<std::thread> ioContextThreads;               ///< Vector of all IO context threads.
};

} // namespace casil

#endif // CASIL_ASIO_H
