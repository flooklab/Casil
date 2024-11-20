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
#include <casil/auxil.h>
#include <casil/bytes.h>
#include <casil/device.h>
#include <casil/TL/directinterface.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/system/errc.hpp>
#include <boost/system/error_code.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

using casil::Device;
using casil::TL::DirectInterface;

using UDPBufferT = std::array<std::uint8_t, 65527>;

namespace boost { using casil::Bytes::operator<<; }

//

#include <boost/test/unit_test.hpp>
#include "../../../datadirfixture.h"

BOOST_FIXTURE_TEST_SUITE(Components_Tests, DataDirFixture)

BOOST_AUTO_TEST_SUITE(UDP_Tests)

BOOST_AUTO_TEST_CASE(Test1_read)
{
    Device d("{transfer_layer: [{name: intf, type: UDP, init: {address: 127.0.0.1, port: 10355}}], hw_drivers: [], registers: []}");

    using boost::asio::ip::udp;
    udp::endpoint endpoint(udp::v4(), 10355);
    udp::socket socket(casil::ASIO::getIOContext(), endpoint);

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        BOOST_REQUIRE(d.init());

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        //Need to receive something first to determine remote endpoint

        udp::endpoint remoteEndpoint(udp::v4(), 10355);

        intf.write({0x99u});
        UDPBufferT readBuffer;
        socket.receive_from(boost::asio::buffer(readBuffer, 1), remoteEndpoint);

        //Now can test read()

        std::vector<std::uint8_t> writeData = {0x30u, 0x31, 0x32, 0x33, 0x34, 'A', 'B'};

        UDPBufferT writeBuffer;

        std::copy(writeData.begin(), writeData.end(), writeBuffer.begin());

        std::size_t n = socket.send_to(boost::asio::buffer(writeBuffer, writeData.size()), remoteEndpoint);

        BOOST_REQUIRE_EQUAL(n, 7);

        writeData = {0x35u};
        std::copy(writeData.begin(), writeData.end(), writeBuffer.begin());

        n = socket.send_to(boost::asio::buffer(writeBuffer, writeData.size()), remoteEndpoint);

        BOOST_REQUIRE_EQUAL(n, 1);

        BOOST_CHECK_EQUAL(intf.read(0), (std::vector<std::uint8_t>{0x30u, 0x31, 0x32, 0x33, 0x34, 'A', 'B'}));
        BOOST_CHECK_EQUAL(intf.read(), (std::vector<std::uint8_t>{0x35}));

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test2_write)
{
    Device d("{transfer_layer: [{name: intf, type: UDP, init: {address: 127.0.0.1, port: 10355}}], hw_drivers: [], registers: []}");

    using boost::asio::ip::udp;
    udp::endpoint endpoint(udp::v4(), 10355);
    udp::socket socket(casil::ASIO::getIOContext(), endpoint);

    std::vector<std::vector<std::uint8_t>> dataChunks;

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        BOOST_REQUIRE(d.init());

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        intf.write({0x30u, 0x31, 0x32, 0x33, 0x34, 'A'});
        intf.write({0x35});

        UDPBufferT readBuffer;

        for (int i = 0; i < 2; ++i)
        {
            std::size_t n = socket.receive(boost::asio::buffer(readBuffer, 65527));
            dataChunks.push_back(std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + n));
        }

        BOOST_CHECK(d.close());
    }

    BOOST_CHECK_EQUAL(dataChunks[0], (std::vector<std::uint8_t>{0x30u, 0x31, 0x32, 0x33, 0x34, 'A'}));
    BOOST_CHECK_EQUAL(dataChunks[1], (std::vector<std::uint8_t>{0x35u}));
}

BOOST_AUTO_TEST_CASE(Test3_query)
{
    Device d("{transfer_layer: [{name: intf, type: UDP, init: {address: 127.0.0.1, port: 10355}}], hw_drivers: [], registers: []}");

    using boost::asio::ip::udp;
    udp::endpoint endpoint(udp::v4(), 10355);
    udp::socket socket(casil::ASIO::getIOContext(), endpoint);

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        BOOST_REQUIRE(d.init());

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        std::size_t readN;
        std::size_t writeN;

        UDPBufferT readBuffer;
        UDPBufferT writeBuffer;

        std::vector<std::uint8_t> writeData = {0xFFu, 0x00u, 0xAAu, 0x23u, 0x24u, 'C'};
        std::copy(writeData.begin(), writeData.end(), writeBuffer.begin());

        bool boostException = false;

        std::thread thrd(
                    [&socket, &readN, &writeN, &readBuffer, &writeBuffer, &writeData, &boostException]()
                    {
                        try
                        {
                            udp::endpoint remoteEndpoint(udp::v4(), 10355);
                            readN = socket.receive_from(boost::asio::buffer(readBuffer, 65527), remoteEndpoint);
                            writeN = socket.send_to(boost::asio::buffer(writeBuffer, writeData.size()), remoteEndpoint);
                        }
                        catch (const boost::system::system_error&)
                        {
                            boostException = true;
                        }
                    });

        const std::vector<std::uint8_t> result = intf.query({0x12u, 0x34, 0x65, 0x87, 0x9A, 0xCB});

        thrd.join();

        BOOST_REQUIRE(boostException == false);

        BOOST_CHECK_EQUAL(readN, 6);

        const auto readData = std::vector<std::uint8_t>(readBuffer.begin(), readBuffer.begin() + readN);

        BOOST_CHECK_EQUAL(readData, (std::vector<std::uint8_t>{0x12u, 0x34u, 0x65u, 0x87u, 0x9Au, 0xCBu}));

        BOOST_REQUIRE_EQUAL(writeN, 6);

        BOOST_CHECK_EQUAL(result, (std::vector<std::uint8_t>{0xFFu, 0x00u, 0xAAu, 0x23u, 0x24u, 'C'}));

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test4_readBufferFunctions)
{
    Device d("{transfer_layer: [{name: intf, type: UDP, init: {address: 127.0.0.1, port: 10355}}], hw_drivers: [], registers: []}");

    using boost::asio::ip::udp;
    udp::endpoint endpoint(udp::v4(), 10355);
    udp::socket socket(casil::ASIO::getIOContext(), endpoint);

    {
        casil::Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        BOOST_REQUIRE(d.init());

        DirectInterface& intf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        //Need to receive something first to determine remote endpoint

        udp::endpoint remoteEndpoint(udp::v4(), 10355);

        intf.write({0x99u});
        UDPBufferT readBuffer;
        socket.receive_from(boost::asio::buffer(readBuffer, 1), remoteEndpoint);

        //Now can test readBufferEmpty() and clearReadBuffer()

        BOOST_CHECK(intf.readBufferEmpty());

        UDPBufferT writeBuffer;

        std::vector<std::uint8_t> writeData = {0x30u, 0x31, 0x32, 0x33, 0x34};
        std::copy(writeData.begin(), writeData.end(), writeBuffer.begin());

        std::size_t n = socket.send_to(boost::asio::buffer(writeBuffer, writeData.size()), remoteEndpoint);

        BOOST_REQUIRE_EQUAL(n, 5);

        BOOST_CHECK(intf.readBufferEmpty() == false);

        intf.clearReadBuffer();

        BOOST_CHECK(intf.readBufferEmpty());

        writeData = {0x13u, 0x54u};
        std::copy(writeData.begin(), writeData.end(), writeBuffer.begin());

        n = socket.send_to(boost::asio::buffer(writeBuffer, writeData.size()), remoteEndpoint);

        BOOST_REQUIRE_EQUAL(n, 2);

        BOOST_CHECK(intf.readBufferEmpty() == false);

        BOOST_CHECK_EQUAL(intf.read(), (std::vector<std::uint8_t>{0x13u, 0x54u}));

        BOOST_CHECK(intf.readBufferEmpty());

        BOOST_CHECK(d.close());
    }
}

BOOST_AUTO_TEST_CASE(Test5_requireIOContextThreads)
{
    Device d("{transfer_layer: [{name: intf, type: UDP, init: {address: 127.0.0.1, port: 10355}}], hw_drivers: [], registers: []}");

    using boost::asio::ip::udp;
    udp::endpoint endpoint(udp::v4(), 10355);
    udp::socket socket(casil::ASIO::getIOContext(), endpoint);
    (void)socket;

    BOOST_CHECK(d.init() == false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
