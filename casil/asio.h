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

class ASIO
{
public:
    ASIO() = delete;
    //
    static boost::asio::io_context& getIOContext();
    //
    static bool startRunIOContext(unsigned int pNumThreads = 1);
    static void stopRunIOContext();
    //
    static bool ioContextThreadsRunning();

private:
    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    static WorkGuard& getWorkGuard();

private:
    static bool ioContextRunning;
    static std::vector<std::thread> ioContextThreads;
};

} // namespace casil

#endif // CASIL_ASIO_H
