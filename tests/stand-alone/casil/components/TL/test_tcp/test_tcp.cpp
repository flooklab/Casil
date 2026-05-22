/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2026 M. Frohne
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

#include <casil/asio.h>
#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/TL/directinterface.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/error_code.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
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

BOOST_AUTO_TEST_CASE(Test1_configValues)
{
    //Valid sets of init options
    const std::vector<std::string> optsStrs1 = {"address: 127.0.0.1, port: 10354, read_termination: \"\\n\"",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\"",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\\n\", write_termination: \"\\r\"",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\", "
                                                    "connect_timeout: 0.0, timeout: 0.0, write_timeout: 0.0",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\", "
                                                    "connect_timeout: 3, timeout: 1.2, write_timeout: 5.0"};

    int numOptsOk = 0;

    for (const auto& optsStr : optsStrs1)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: TCP, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            continue;   //Skip incrementing success counter on error
        }
        ++numOptsOk;
    }

    BOOST_CHECK_EQUAL(numOptsOk, optsStrs1.size());

    //Invalid sets of init options
    const std::vector<std::string> optsStrs2 = {"port: 10354, read_termination: \"\\n\"",
                                                "address: \"\", port: 10354, read_termination: \"\\n\"",
                                                "address: 127.0.0.1, read_termination: \"\\n\"",
                                                "address: 127.0.0.1, port: 0, read_termination: \"\\n\"",
                                                "address: 127.0.0.1, port: 10354",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\", connect_timeout: -2",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\", timeout: -1.0",
                                                "address: 127.0.0.1, port: 10354, read_termination: \"\", write_timeout: -0.1"};

    int numOptsErr = 0;

    for (const auto& optsStr : optsStrs2)
    {
        try
        {
            Device d("{transfer_layer: [{name: intf, type: TCP, init: {" + optsStr + "}}], hw_drivers: [], registers: []}");
            (void)d;
        }
        catch (const std::runtime_error&)
        {
            ++numOptsErr;    //Increment error counter on error
        }
    }

    BOOST_CHECK_EQUAL(numOptsErr, optsStrs2.size());
}

BOOST_AUTO_TEST_CASE(Test2_read)
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

BOOST_AUTO_TEST_CASE(Test3_write)
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

BOOST_AUTO_TEST_CASE(Test4_query)
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

BOOST_AUTO_TEST_CASE(Test5_readBufferFunctions)
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

BOOST_AUTO_TEST_CASE(Test6_requireIOContextThreads)
{
    Device d("{transfer_layer: [{name: intf, type: TCP,"
                                "init: {address: 127.0.0.1, port: 10354, read_termination: \"\\n\"}}],"
              "hw_drivers: [], registers: []}");

    std::atomic_bool handlerError(false);
    std::atomic_bool handlerCalled(false);

    auto handleAccept = [&handlerError, &handlerCalled](const boost::system::error_code& pErrorCode)
    {
        if (pErrorCode.value() != boost::system::errc::success)
            handlerError.store(true);

        handlerCalled.store(true);
    };

    using boost::asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), 10354);
    tcp::acceptor acceptor(casil::ASIO::getIOContext(), endpoint, false);
    tcp::socket socket(casil::ASIO::getIOContext());

    acceptor.async_accept(socket, handleAccept);

    BOOST_CHECK(d.init() == false);

    acceptor.cancel();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    BOOST_CHECK(handlerError.load() == false);
    BOOST_CHECK(handlerCalled.load() == false);

    //Despite previous cancelling, handler will be called from future IO context threads;
    //hence need to make it being called now to prevent crashes in other tests

    casil::Auxil::AsyncIORunner<1> ioRunner;
    (void)ioRunner;

    for (int i = 0; i < 20; ++i)
    {
        if (handlerCalled.load())
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    BOOST_CHECK(handlerError.load() == true);
    BOOST_CHECK(handlerCalled.load() == true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
