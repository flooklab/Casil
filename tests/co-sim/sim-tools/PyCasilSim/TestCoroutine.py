##################################################################################################
#
#  Copyright (C) 2026 M. Frohne
#
#  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
#
#  Casil is free software: you can redistribute it and/or modify it
#  under the terms of the GNU Affero General Public License as published
#  by the Free Software Foundation, either version 3 of the License,
#  or (at your option) any later version.
#
#  Casil is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with Casil. If not, see <https://www.gnu.org/licenses/>.
#
##################################################################################################
#
#  This file has been directly adapted from the corresponding code of
#  the basil software, which itself is covered by the following license:
#
#      Copyright (C) 2011–2026 SiLab, Institute of Physics, University of Bonn
#
#      All rights reserved.
#
#      Redistribution and use in source and binary forms, with or without
#      modification, are permitted provided that the following conditions are
#      met:
#
#       *  Redistributions of source code must retain the above copyright notice,
#          this list of conditions and the following disclaimer.
#
#       *  Redistributions in binary form must reproduce the above copyright
#          notice, this list of conditions and the following disclaimer in the
#          documentation and/or other materials provided with the distribution.
#
#       *  Neither the name of the copyright holder nor the names of its
#          contributors may be used to endorse or promote products derived from
#          this software without specific prior written permission.
#
#      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#      IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
#      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#      HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#      TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##################################################################################################
#
#  Initial version by Chris Higgs <chris.higgs@potentialventures.com>
#
##################################################################################################

from .MessageInterface import MessageInterface

import PyCasil
MessageType = PyCasil.Layers.TL.SiSim.MessageType

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import Timer

import importlib
import logging
import os
import socket
import time
import yaml

def _getBusDriver():
    """Get the bus driver to be used as imported module."""
    return _importModuleElem(os.getenv("SIMULATION_BUS", "PyCasilSim.BusDrivers.BasilBusDriver"))

def _importModuleElem(qualifiedName):
    """Return 'X' as would be obtained by 'from a.b.c.X import X'."""
    return getattr(importlib.import_module(qualifiedName), qualifiedName.split(".")[-1])

@cocotb.test(skip=False)
async def test(dut, debug=False):
    """Forward between SiSim interface from test case and simulation bus by passing messages over a TCP socket."""

    logger = logging.getLogger("Co-Simulation")
    if debug:
        logger.setLevel(logging.DEBUG)
    else:
        logger.setLevel(logging.INFO)

    host = os.getenv("SIMULATION_HOST", "localhost")
    port = os.getenv("SIMULATION_PORT", "12345")
    busClkFreq = int(os.getenv("SIMULATION_BUS_CLK_PERIOD", "5000"))
    busTransactionWait = int(os.getenv("SIMULATION_TRANSACTION_WAIT", "50000"))
    busClock = bool(int(os.getenv("SIMULATION_BUS_CLOCK", "1")))

    bus = _getBusDriver()(dut)

    logger.info(f"Used bus driver: {type(bus).__name__}")
    logger.info(f"Bus clock: {busClock}")
    logger.info(f"Bus clock period: {busClkFreq}")
    logger.info(f"Bus transaction wait: {busTransactionWait}")

    simModules = []
    simModulesYAMLStr = os.getenv("SIMULATION_MODULES", "")
    if simModulesYAMLStr:
        simModulesYAMLDoc = yaml.safe_load(simModulesYAMLStr)
        for mod in simModulesYAMLDoc:
            modImport = _importModuleElem(mod)
            kargs = dict(simModulesYAMLDoc[mod])
            simModules.append(modImport(dut, **kargs))
            logger.info("Using simulation modules: %s (arguments: %s)", mod, kargs)

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    try:
        sock.bind((host, int(port)))
        sock.listen(1)
    except Exception:
        sock.close()
        sock = None
        raise

    #Start a clock generator
    if busClock:
        cocotb.start_soon(Clock(bus.clock, busClkFreq).start())

    #Start additional simulation modules
    for mod in simModules:
        cocotb.start_soon(mod.run())

    await bus.init()

    while True:
        logger.info("Waiting for incoming connection on %s:%d...", host, int(port))
        sockConn, connAddr = sock.accept()
        logger.info("Accepted connection from %s:%d.", connAddr[0], connAddr[1])
        msgIntf = MessageInterface(sockConn)

        while True:
            #Uncomment for constantly advancing clock:
            # await RisingEdge(bus.clock)

            try:
                msgType, reqField1, reqField2 = msgIntf.tryReceiveRequest()
            except EOFError:
                logger.info("Remote server closed the connection.")
                sockConn.shutdown(socket.SHUT_RDWR)
                sockConn.close()
                break
            if msgType is None:
                time.sleep(0)
                continue

            #Uncomment to add a few clock cycles:
            # for _ in range(10):
            #     await RisingEdge(bus.clock)

            await Timer(busTransactionWait)

            if msgType == MessageType.WriteRequest:
                reqAddr, reqData = reqField1, reqField2
                logger.debug("Received a WriteRequest: 0x%04x <- %s", reqAddr, reqData)
                await bus.write(reqAddr, reqData)
            elif msgType == MessageType.ReadRequest:
                reqAddr, reqSize = reqField1, reqField2
                logger.debug("Received a ReadRequest: 0x%04x (size %d)", reqAddr, reqSize)
                result = await bus.read(reqAddr, reqSize)
                logger.debug("Send a ReadResponse: %s", str(result))
                msgIntf.sendResponse(result)
            else:
                raise RuntimeError("Invalid message type: %s" % str(msgType))

            #Uncomment to add a few clock cycles:
            # for _ in range(10):
            #     await RisingEdge(bus.clock)

            await Timer(busTransactionWait)

        if os.getenv("SIMULATION_END_ON_DISCONNECT"):
            break

    sock.shutdown(socket.SHUT_RDWR)
    sock.close()
