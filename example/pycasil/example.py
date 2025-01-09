##################################################################################################
#
# This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
# Copyright (C) 2024–2025 M. Frohne
#
# Casil is free software: you can redistribute it and/or modify it
# under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# Casil is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Casil. If not, see <https://www.gnu.org/licenses/>.
#
##################################################################################################

# Test the serial port interface:
# - Run "socat -d -d pty,raw,echo=0,b19200 pty,raw,echo=0,b19200" in a first terminal
# - Run this script in a second terminal

import PyCasil as pcs

pcs.Logger.setLogLevel(pcs.Logger.LogLevel.Debug)
pcs.Logger.addOutputCout()

pcs.Logger.log("HelloWorld0", pcs.Logger.LogLevel.Error)
pcs.Logger.log("HelloWorld1", pcs.Logger.LogLevel.Info)
pcs.Logger.log("HelloWorld2", pcs.Logger.LogLevel.DebugDebug)

pcs.Logger.logInfo("Casil version: " + pcs.Version.toString())

with pcs.Auxil.AsyncIORunner(2):

    d = pcs.Device("{transfer_layer: [{name: intf, type: Serial,"
                                      "init: {port: /dev/pts/2, read_termination: \"\\n\\r\", baudrate: 19200}},"
                                     "{name: intf2, type: Serial,"
                                      "init: {port: /dev/pts/3, read_termination: \"\\n\\r\", baudrate: 19200}}],"
                    "hw_drivers: [{name: echo, type: VirtEcho, interface: intf2}],"
                    "registers: []}")

    d.init()

    d.interface("intf").write([0x30, 0x31, 0x32, 0x33, 0x34, ord('A')])
    d.interface("intf").write([0x35])

    d.driver("echo")(1)
    d.driver("echo")(-1)
    d.driver("echo")(-1)

    for it in d.interface("intf").read(1):
        pcs.Logger.logInfo("Byte0 " + str(it))
    for it in d.interface("intf").read():
        pcs.Logger.logInfo("Bytes " + str(it))
    for it in d.interface("intf").read(1):
        pcs.Logger.logInfo("Byte0 " + str(it))
    for it in d.interface("intf").read():
        pcs.Logger.logInfo("Bytes " + str(it))

    d.close()
