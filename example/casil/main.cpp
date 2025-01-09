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

#include <casil/auxil.h>
#include <casil/device.h>
#include <casil/logger.h>
#include <casil/version.h>
#include <casil/HL/Direct/virtecho.h>
#include <casil/TL/directinterface.h>

#include <string>

using casil::Device;
using casil::Logger;

using casil::TL::DirectInterface;
using casil::HL::VirtEcho;

namespace Auxil = casil::Auxil;
namespace Version = casil::Version;

//Test the serial port interface:
// - Run "socat -d -d pty,raw,echo=0,b19200 pty,raw,echo=0,b19200" in a first terminal
// - Run this example in a second terminal

int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

    Logger::setLogLevel(Logger::LogLevel::Debug);
    Logger::addOutputCout();

    Logger::log("HelloWorld0", Logger::LogLevel::Error);
    Logger::log("HelloWorld1", Logger::LogLevel::Info);
    Logger::log("HelloWorld2", Logger::LogLevel::DebugDebug);

    Logger::log("Casil version: " + Version::toString(), Logger::LogLevel::Info);

    Device d("{transfer_layer: [{name: intf, type: Serial,"
                                "init: {port: /dev/pts/2, read_termination: \"\\n\\r\", baudrate: 19200}},"
                               "{name: intf2, type: Serial,"
                                "init: {port: /dev/pts/3, read_termination: \"\\n\\r\", baudrate: 19200}}],"
              "hw_drivers: [{name: echo, type: VirtEcho, interface: intf2}],"
              "registers: []}");

    {
        Auxil::AsyncIORunner<2> ioRunner;
        (void)ioRunner;

        if (!d.init())
            return 1;

        DirectInterface& serIntf = dynamic_cast<DirectInterface&>(d.interface("intf"));

        serIntf.write({0x30u, 0x31, 0x32, 0x33, 0x34, 'A'});
        serIntf.write({0x35});

        VirtEcho& echoDrv = dynamic_cast<VirtEcho&>(d.driver("echo"));

        echoDrv(1);
        echoDrv(-1);
        echoDrv(-1);

        for (std::uint8_t it : serIntf.read(1))
            Logger::logInfo("Byte0 " + std::to_string(it));
        for (std::uint8_t it : serIntf.read())
            Logger::logInfo("Bytes " + std::to_string(it));
        for (std::uint8_t it : serIntf.read(1))
            Logger::logInfo("Byte0 " + std::to_string(it));
        for (std::uint8_t it : serIntf.read())
            Logger::logInfo("Bytes " + std::to_string(it));

        d.close();
    }

    return 0;
}
