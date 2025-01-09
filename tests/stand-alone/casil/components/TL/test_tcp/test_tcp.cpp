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
#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/TL/directinterface.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

using casil::Device;
using casil::TL::DirectInterface;

namespace boost { using casil::Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(TCP_Tests)

BOOST_AUTO_TEST_CASE(Test1_read)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerCompleted(false);
    std::atomic_bool handlerError(false);

    auto handleAccept = [&handlerCompleted, &handlerError](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);

        handlerCompleted.store(true);
        handlerCompleted.notify_one();
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        acceptor.async_accept(socket, handleAccept);

        BOOST_REQUIRE(d.init());

        handlerCompleted.wait(false);

        BOOST_CHECK(handlerError.load() == false);

        std::vector<std::uint8_t> writeBuffer;

        writeBuffer = {0x30u, 0x31, 0x32, 0x33, 0x34, 'A', '\n'};

        std::size_t n = boost::asio::write(socket, boost::asio::buffer(writeBuffer, writeBuffer.size()));

        BOOST_REQUIRE_EQUAL(n, 7);

        writeBuffer = {0x35u, '\n'};

        n = boost::asio::write(socket, boost::asio::buffer(writeBuffer, writeBuffer.size()));

        BOOST_REQUIRE_EQUAL(n, 2);

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        BOOST_CHECK_EQUAL(intf.read(0), (std::vector<std::uint8_t>{}));
        BOOST_CHECK_EQUAL(intf.read(3), (std::vector<std::uint8_t>{0x30u, 0x31, 0x32}));
        BOOST_CHECK_EQUAL(intf.read(-1), (std::vector<std::uint8_t>{0x33, 0x34, 'A'}));
        BOOST_CHECK_EQUAL(intf.read(-1), (std::vector<std::uint8_t>{0x35}));

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test2_write)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerCompleted(false);
    std::atomic_bool handlerError(false);

    auto handleAccept = [&handlerCompleted, &handlerError](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);

        handlerCompleted.store(true);
        handlerCompleted.notify_one();
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    std::vector<std::vector<std::uint8_t>> dataChunks;

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        acceptor.async_accept(socket, handleAccept);

        BOOST_REQUIRE(d.init());

        handlerCompleted.wait(false);

        BOOST_CHECK(handlerError.load() == false);

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        intf.write({0x30u, 0x31, 0x32, 0x33, 0x34, 'A'});
        intf.write({0x35});

        std::vector<std::uint8_t> readBuffer;

        for (int i = 0; i < 2; ++i)
        {
            std::size_t n = boost::asio::read_until(socket, boost::asio::dynamic_buffer(readBuffer), "\n");

            dataChunks.push_back(std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + n));

            readBuffer.erase(readBuffer.begin(), readBuffer.begin() + n);
        }

        BOOST_CHECK(d.close());
    }

    BOOST_CHECK_EQUAL(dataChunks[0], (std::vector<std::uint8_t>{0x30u, 0x31, 0x32, 0x33, 0x34, 'A', '\n'}));
    BOOST_CHECK_EQUAL(dataChunks[1], (std::vector<std::uint8_t>{0x35u, '\n'}));
}

BOOST_AUTO_TEST_CASE(Test3_query)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerCompleted(false);
    std::atomic_bool handlerError(false);

    auto handleAccept = [&handlerCompleted, &handlerError](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);

        handlerCompleted.store(true);
        handlerCompleted.notify_one();
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        acceptor.async_accept(socket, handleAccept);

        BOOST_REQUIRE(d.init());

        handlerCompleted.wait(false);

        BOOST_CHECK(handlerError.load() == false);

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        std::size_t readN;
        std::size_t writeN;

        std::vector<std::uint8_t> readBuffer;
        std::vector<std::uint8_t> writeBuffer = {0xFFu, 0x00u, 0xAAu, 0x23u, 0x24u, '\n'};

        bool boostException = false;

        std::thread thrd(
                    [&socket, &readN, &writeN, &readBuffer, &writeBuffer, &boostException]()
                    {
                        try
                        {
                            readN = boost::asio::read_until(socket, boost::asio::dynamic_buffer(readBuffer), "\n");
                            writeN = boost::asio::write(socket, boost::asio::buffer(writeBuffer, writeBuffer.size()));
                        }
                        catch (const boost::system::system_error&)
                        {
                            boostException = true;
                        }
                    });

        const std::vector<std::uint8_t> result = intf.query({0x12u, 0x34, 0x65, 0x87, 0x9A, 0xCB}, 5);

        thrd.join();

        BOOST_REQUIRE(boostException == false);

        BOOST_CHECK_EQUAL(readN, 7);
        BOOST_CHECK_EQUAL(readBuffer, (std::vector<std::uint8_t>{0x12u, 0x34u, 0x65u, 0x87u, 0x9Au, 0xCBu, '\n'}));

        BOOST_REQUIRE_EQUAL(writeN, 6);

        BOOST_CHECK_EQUAL(result, (std::vector<std::uint8_t>{0xFFu, 0x00u, 0xAAu, 0x23u, 0x24u}));

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test4_readBufferFunctions)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerCompleted(false);
    std::atomic_bool handlerError(false);

    auto handleAccept = [&handlerCompleted, &handlerError](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);

        handlerCompleted.store(true);
        handlerCompleted.notify_one();
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        acceptor.async_accept(socket, handleAccept);

        BOOST_REQUIRE(d.init());

        handlerCompleted.wait(false);

        BOOST_CHECK(handlerError.load() == false);

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        BOOST_CHECK(intf.readBufferEmpty());

        std::vector<std::uint8_t> writeBuffer;
        writeBuffer = {0x30u, 0x31, 0x32, 0x33, 0x34};
        std::size_t n = boost::asio::write(socket, boost::asio::buffer(writeBuffer, writeBuffer.size()));

        BOOST_REQUIRE_EQUAL(n, 5);

        BOOST_CHECK(intf.readBufferEmpty() == false);

        intf.clearReadBuffer();

        BOOST_CHECK(intf.readBufferEmpty());

        writeBuffer = {0x13u, 0x54u};
        n = boost::asio::write(socket, boost::asio::buffer(writeBuffer, writeBuffer.size()));

        BOOST_REQUIRE_EQUAL(n, 2);

        BOOST_CHECK(intf.readBufferEmpty() == false);

        BOOST_CHECK_EQUAL(intf.read(2), (std::vector<std::uint8_t>{0x13u, 0x54u}));

        BOOST_CHECK(intf.readBufferEmpty());

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test5_requireIOContextThreads)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerError(false);

    auto handleAccept = [&handlerError](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    acceptor.async_accept(socket, handleAccept);

    BOOST_CHECK(d.init() == false);

    acceptor.cancel();

    BOOST_REQUIRE(handlerError.load() == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
