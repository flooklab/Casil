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

from PyCasilSim.CocotbSimSetup import compileAndRunSim, cleanUp

import PyCasil as pcs

import os
import unittest

class PyCasilTestCase(unittest.TestCase):
    """Common base class for all test cases, which runs the simulation and initializes a device for the given YAML config."""

    __test__ = False    #Do not include this common base class in pytest's test case lookup

    def __init__(self, methodName, testFile, testBenchFiles, yamlConfig, busSplit=False, **kwargs):
        """Pass methodName to unittest.TestCase and save the other arguments for use by setUp()."""
        super().__init__(methodName)
        self.testFile = testFile
        self.testBenchFiles = testBenchFiles
        self.yamlConfig = yamlConfig

        busDrv="PyCasilSim.BusDrivers.BasilBusDriver"
        busSplitDef = ()
        if busSplit is not False:
            busDrv="PyCasilSim.BusDrivers.BasilSbusDriver"
            if busSplit == "sbus":
                busSplitDef = ("BASIL_SBUS",)
            elif busSplit == "top":
                busSplitDef = ("BASIL_TOPSBUS",)

        if "simBus" not in kwargs.keys():
            kwargs["simBus"] = busDrv
        if "extraDefines" not in kwargs.keys():
            kwargs["extraDefines"] = busSplitDef
        else:
            kwargs["extraDefines"] += busSplitDef

        self.kwargs = kwargs

    def setUp(self):
        """
        Compile and start the cocotb simulation and prepare a DUT instance.

        Configures the PyCasil logger, compiles and runs the cocotb simulation, starts an IO thread,
        creates a device instance (using the 'yamlConfig' argument from __init__()) and initializes it.

        Passes the 'testBenchFiles' as well as 'kwargs' arguments from __init__() to the simulation script.
        """
        pcs.Logger.setLogLevel(pcs.Logger.LogLevel.Info)
        pcs.Logger.addOutputCout()

        compileAndRunSim([os.path.join(os.path.dirname(self.testFile), testBenchFile) for testBenchFile in self.testBenchFiles],
                         **self.kwargs)

        pcs.ASIO.startRunIOContext(1)

        self.device = pcs.Device(self.yamlConfig)
        if not self.device.init():
            raise RuntimeError("Could not initialize chip.")

    def tearDown(self):
        """Close device and clean up temporary files."""
        self.device.close()
        pcs.ASIO.stopRunIOContext()
        cleanUp()
