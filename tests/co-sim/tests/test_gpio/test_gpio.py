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

from PyCasilSim.TestCase import PyCasilTestCase

import pytest
import unittest

configYAML = """
transfer_layer:
  - name  : INTF
    type  : SiSim
    init:
        host : localhost
        port  : 12345

hw_drivers:
  - name      : GPIO
    type      : gpio
    interface : INTF
    base_addr : 0x0000
    size      : 24

  - name      : GPIO2
    type      : gpio
    interface : INTF
    base_addr : 0x0010
    size      : 16

registers:
  - name        : GPIO_REG
    type        : StdRegister
    hw_driver   : GPIO
    size        : 24
    fields:
      - name    : OUT
        size    : 8
        offset  : 7
      - name    : IN
        size    : 8
        offset  : 15
      - name    : TRI_IN
        size    : 4
        offset  : 19
      - name    : TRI_OUT
        size    : 4
        offset  : 23
"""

class TestSimGPIO(PyCasilTestCase):
    __test__ = True

    def __init__(self, methodName="runTest", busSplit=False):
        super().__init__(methodName=methodName, testFile=__file__, testBenchFiles=["test_gpio.v"], yamlConfig=configYAML, busSplit=busSplit)

    def setUp(self):
        super().setUp()

    def test_io(self):
        self.device["GPIO"].setOutputEn([0xFF, 0, 0])   #To remove 'z in simulation

        ret = self.device["GPIO"].getData()
        self.assertEqual([0, 0, 0], ret)

        self.device["GPIO"].setOutputEn([0x0F, 0, 0])
        self.device["GPIO"].setData([0xE3, 0xFA, 0x5A])
        ret = self.device["GPIO"].getData()
        self.assertEqual([0x33, 0x5A, 0x5A], ret)
        ret = self.device["GPIO2"].getData()
        self.assertEqual([0xA5, 0xCD], ret)

    def test_ioRegister(self):
        self.device["GPIO"].setOutputEn([0xFF, 0, 0])   #To remove 'z in simulation

        self.device["GPIO_REG"]["OUT"] = 0xA5

        self.device["GPIO_REG"].write()
        ret = self.device["GPIO"].getData()
        self.assertEqual([0, 0xA5, 0xA5], ret)
        #TODO: Add register readback and comparison

    def tearDown(self):
        super().tearDown()

@pytest.mark.verilator
class TestSimGPIOSbus(TestSimGPIO):
    def __init__(self, methodName="runTest"):
        super().__init__(methodName=methodName, busSplit="sbus")

@pytest.mark.verilator
class TestSimGPIOSbusTop(TestSimGPIO):
    def __init__(self, methodName="runTest"):
        super().__init__(methodName=methodName, busSplit="top")

if __name__ == "__main__":
    unittest.main()
