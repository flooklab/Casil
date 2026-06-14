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

from cocotb.triggers import RisingEdge, Timer
from cocotb.types import LogicArray
from cocotb_bus.drivers import BusDriver

class BasilBusDriver(BusDriver):
    """Bus driver abstraction for the *regular* "basil" bus."""

    _signals = ["BUS_CLK", "BUS_RST", "BUS_DATA", "BUS_ADD", "BUS_RD", "BUS_WR"]
    _optional_signals = ["BUS_BYTE_ACCESS"]

    def __init__(self, entity):
        """Initialize cocotb bus driver with bus handle 'entity' from simulation."""
        BusDriver.__init__(self, entity, "", entity.BUS_CLK, case_insensitive=False)

        #Create an appropriately sized high-impedance value for BUS_DATA
        self._highImpedance = LogicArray("Z" * len(self.bus.BUS_DATA))

        #Create an appropriately sized "unknown" value for BUS_ADD
        self._xVal = LogicArray("x" * len(self.bus.BUS_ADD))

        self._hasByteAccess = False

    async def init(self):
        """Initialize and reset the bus."""
        self.bus.BUS_RST.value = 1
        self.bus.BUS_RD.value = 0
        self.bus.BUS_WR.value = 0
        self.bus.BUS_ADD.value = self._xVal
        self.bus.BUS_DATA.value = self._highImpedance

        for _ in range(8):
            await RisingEdge(self.clock)

        self.bus.BUS_RST.value = 0

        for _ in range(2):
            await RisingEdge(self.clock)

        try:
            getattr(self.bus, "BUS_BYTE_ACCESS")
        except Exception:
            self._hasByteAccess = False
        else:
            self._hasByteAccess = True

    async def read(self, address, size):
        """Read 'size' bytes from bus, starting from bus address 'address'."""
        result = []

        self.bus.BUS_RD.value = 0
        self.bus.BUS_ADD.value = self._xVal
        self.bus.BUS_DATA.value = self._highImpedance

        await RisingEdge(self.clock)

        if size == 0:
            return result

        byte = 0

        while byte <= size:
            if byte == size:
                self.bus.BUS_RD.value = 0
            else:
                self.bus.BUS_RD.value = 1

            self.bus.BUS_ADD.value = address + byte

            await RisingEdge(self.clock)

            if byte != 0:
                if self._hasByteAccess and self.bus.BUS_BYTE_ACCESS.value == 0:
                    result.append(self.bus.BUS_DATA.value.to_unsigned() & 0x000000FF)
                    result.append((self.bus.BUS_DATA.value.to_unsigned() & 0x0000FF00) >> 8)
                    result.append((self.bus.BUS_DATA.value.to_unsigned() & 0x00FF0000) >> 16)
                    result.append((self.bus.BUS_DATA.value.to_unsigned() & 0xFF000000) >> 24)
                else:
                    if len(self.bus.BUS_DATA.value) == 8:
                        result.append(self.bus.BUS_DATA.value.to_unsigned() & 0xFF)
                    else:
                        result.append(self.bus.BUS_DATA.value[7:0].to_unsigned() & 0xFF)

            if self._hasByteAccess and self.bus.BUS_BYTE_ACCESS.value == 0:
                byte += 4
            else:
                byte += 1

        self.bus.BUS_ADD.value = self._xVal
        self.bus.BUS_DATA.value = self._highImpedance

        await RisingEdge(self.clock)

        return result

    async def write(self, address, data):
        """Write data bytes 'data' to bus, starting from bus address 'address'."""
        self.bus.BUS_ADD.value = self._xVal
        self.bus.BUS_DATA.value = self._highImpedance
        self.bus.BUS_WR.value = 0

        await RisingEdge(self.clock)

        for index, byte in enumerate(data):
            self.bus.BUS_DATA.value = byte
            self.bus.BUS_WR.value = 1
            self.bus.BUS_ADD.value = address + index
            await Timer(1)  #This is a hack for iverilog
            self.bus.BUS_DATA.value = byte
            self.bus.BUS_WR.value = 1
            self.bus.BUS_ADD.value = address + index

            await RisingEdge(self.clock)

        if self._hasByteAccess and self.bus.BUS_BYTE_ACCESS.value == 0:
            raise NotImplementedError("BUS_BYTE_ACCESS is not implemented for write().")

        self.bus.BUS_DATA.value = self._highImpedance
        self.bus.BUS_ADD.value = self._xVal
        self.bus.BUS_WR.value = 0

        await RisingEdge(self.clock)
