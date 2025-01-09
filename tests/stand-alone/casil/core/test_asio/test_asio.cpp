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

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <chrono>
#include <thread>

//

#include <boost/test/unit_test.hpp>
#include "../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Core_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(Asio_Tests)

BOOST_AUTO_TEST_CASE(Test1_asyncIOContext)
{
    using casil::ASIO;

    BOOST_CHECK(ASIO::startRunIOContext(0) == false);
    BOOST_CHECK(ASIO::startRunIOContext(1) == true);
    BOOST_CHECK(ASIO::startRunIOContext(2) == false);

    ASIO::stopRunIOContext();

    BOOST_REQUIRE(ASIO::startRunIOContext(2) == true);

    std::atomic_bool accepted(false);

    auto handleAccept1 = [&accepted](const boost::system::error_code& pError)
    {
        if (!pError)
            accepted.store(true);
    };

    //Accept connection
    boost::asio::ip::tcp::endpoint endpoint1(boost::asio::ip::tcp::v4(), 10354);
    boost::asio::ip::tcp::acceptor acceptor1(ASIO::getIOContext(), endpoint1, false);
    boost::asio::ip::tcp::socket socket1(ASIO::getIOContext());
    acceptor1.async_accept(socket1, handleAccept1);

    //Start connection
    boost::asio::ip::tcp::socket socket2(ASIO::getIOContext());
    boost::asio::ip::tcp::resolver resolver2(ASIO::getIOContext());
    boost::asio::ip::tcp::resolver::query query2("127.0.0.1", "10354");
    boost::asio::connect(socket2, resolver2.resolve(query2));

    for (int i = 0; i < 10; ++i)
    {
        if (accepted.load())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    BOOST_CHECK(accepted.load());

    socket2.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket2.close();

    ASIO::stopRunIOContext();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
