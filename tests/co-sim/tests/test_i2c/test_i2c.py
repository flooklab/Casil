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

import unittest

configYAML = """
transfer_layer:
  - name  : INTF
    type  : SiSim
    init:
        host : localhost
        port  : 12345

hw_drivers:
  - name      : i2c
    type      : i2c
    interface : INTF
    base_addr : 0x1000

registers:
  - name        : CONTROL
    type        : StdRegister
    hw_driver   : i2c
    size        : 8
    fields:
      - name    : OUT
        size    : 8
        offset  : 7
"""

class TestSimI2C(PyCasilTestCase):
    __test__ = True

    def __init__(self, methodName="runTest"):
        super().__init__(methodName=methodName, testFile=__file__, testBenchFiles=["test_i2c.v"], yamlConfig=configYAML)

    def setUp(self):
        super().setUp()

    def test_i2c(self):
        data = [0x85, 0x81, 0xA5, 0x91]
        self.device["i2c"].sendWrite(0x92, data)

        ret = self.device["i2c"].getData(4)
        self.assertEqual(ret, data)

        self.device["i2c"].sendWrite(0x92, data[0:1])

        self.device["i2c"].setData([0, 1, 2, 3])

        ret = self.device["i2c"].sendRead(0x92, 3)
        self.assertEqual(ret, data[1:])

        self.device["i2c"].sendWrite(0x92, range(16))
        self.device["i2c"].sendWrite(0x92, [0])
        ret = self.device["i2c"].sendRead(0x92, 15)
        self.assertEqual(ret, list(range(16))[1:])

        #Expect exception due to NO_ACK for invalid I2C address
        excRaised = False
        try:
            self.device["i2c"].sendWrite(0x55, data)
        except RuntimeError:
            excRaised = True
        self.assertEqual(excRaised, True)

    def tearDown(self):
        super().tearDown()

if __name__ == "__main__":
    unittest.main()
