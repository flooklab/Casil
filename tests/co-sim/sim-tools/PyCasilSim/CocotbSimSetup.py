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

import os
import pathlib
import subprocess

def compileAndRunSim(*args, **kwargs):
    """Generate a cocotb Makefile and run make (i.e. simulation) as background process."""
    with open("Makefile", "w") as f:
        f.write(_generateMakefile(*args, **kwargs))
    subprocess.Popen(["make"])  #Run simulator in the background

def cleanUp():
    """Let the Makefile clean up and finally remove the Makefile."""
    subprocess.run(["make", "clean"])
    subprocess.run(["rm", "-f", "Makefile"])

def _getBasilFirmwareDir():
    """Get the firmware directory."""
    if "BASIL_FIRMWARE_DIR" in os.environ:
        return os.environ["BASIL_FIRMWARE_DIR"]
    else:
        return str(pathlib.Path(__file__).parent.parent.parent.parent.parent.joinpath("external/basil/basil/firmware").absolute())

def _generateMakefile(
    simFiles,
    topLevel="tb",
    testModule="PyCasilSim.TestCoroutine",
    simHost="localhost",
    simPort=12345,
    simBus="PyCasilSim.BusDrivers.BasilBusDriver",
    endOnDisconnect=True,
    includeDirs=(),
    extraDefines=(),
    compileArgs=(),
    buildArgs=(),
    extra=""
):
    """Generate a cocotb Makefile for co-simulating a Verilog testbench (using different possible simulators) alongside a test coroutine."""

    firmwareDir = _getBasilFirmwareDir()
    includeDirs += (firmwareDir + "/modules", firmwareDir + "/modules/includes")

    mkfile = "SIMULATION_HOST?=%s\nSIMULATION_PORT?=%d\nSIMULATION_BUS?=%s\n" % (simHost, simPort, simBus)

    if endOnDisconnect:
        mkfile += "SIMULATION_END_ON_DISCONNECT?=1\n"

    mkfile += "\n"

    mkfile += "VERILOG_SOURCES = %s\n\n" % (" ".join(os.path.abspath(str(e)) for e in simFiles))

    mkfile += "TOPLEVEL = %s\nCOCOTB_TEST_MODULES = %s\n\n" % (topLevel, testModule)

    mkfile += "ICARUS_INCLUDE_DIRS = %s\n" % (" ".join("-I" + str(e) for e in includeDirs))
    mkfile += "ICARUS_DEFINES += %s\n\n" % (" ".join("-D" + str(e) for e in extraDefines))

    mkfile += "NOT_ICARUS_DEFINES = %s\n" % (" ".join("+define+" + str(e) for e in extraDefines))
    mkfile += "NOT_ICARUS_INCLUDE_DIRS=+incdir+./ %s\n" % (
        " ".join("+incdir+" + str(e) for e in includeDirs)
    )   #This is for modelsim; TODO better full path?

    mkfile += "VERILATOR_DEFINES = %s\n" % (" ".join("-D" + str(e) for e in extraDefines))
    mkfile += "VERILATOR_INCLUDE_DIRS = %s\n" % (" ".join("-I" + str(e) for e in includeDirs))

    mkfile += "COMPILE_ARGS_DEFINES = %s\n" % (
        " ".join(str(e) for e in compileArgs)
    )   #Extra compiler args; e.g. for adding Xilinx's glbl.v to Icarus use "-s glbl"
    mkfile += "BUILD_ARGS_DEFINES = %s\n" % (
        " ".join(str(e) for e in buildArgs)
    )   #Extra build args passed to build stage in supported simulators

    mkfile += "\n"
    mkfile += extra
    mkfile += "\n"

    try:
        if os.environ["SIM"] == "verilator":
            mkfile += "EXTRA_ARGS += -DVERILATOR_SIM\n"
            mkfile += "EXTRA_ARGS += -Wno-WIDTH -Wno-TIMESCALEMOD -Wwarn-ASSIGNDLY\n"
            #mkfile += "EXTRA_ARGS += -Wno-MULTIDRIVEN\n"
            mkfile += "EXTRA_ARGS += --trace\n"
    except KeyError:
        pass

    mkfile += """
export SIMULATION_HOST
export SIMULATION_PORT
export SIMULATION_BUS
export SIMULATION_END_ON_DISCONNECT

export COCOTB=$(shell cocotb-config --share)
#export COCOTB=$(shell SPHINX_BUILD=1 python -c \
#    "import cocotb; import os; print(os.path.dirname(os.path.dirname(os.path.abspath(cocotb.__file__))))")
#export PYTHONPATH=$(shell python -c "import sysconfig; print(sysconfig.get_path('platlib'))"):$(COCOTB)
#export LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:$(PYTHONLIBS)
#export PYTHONHOME=$(shell python -c "from sysconfig import get_config_var; print(get_config_var('prefix'))")

ifeq ($(SIM), questa)
    EXTRA_ARGS += $(NOT_ICARUS_DEFINES)
    EXTRA_ARGS += $(NOT_ICARUS_INCLUDE_DIRS)
else ifeq ($(SIM), ius)
    EXTRA_ARGS += $(NOT_ICARUS_DEFINES)
    EXTRA_ARGS += $(NOT_ICARUS_INCLUDE_DIRS)
else ifeq ($(SIM), verilator)
    EXTRA_ARGS += $(VERILATOR_DEFINES)
    EXTRA_ARGS += $(VERILATOR_INCLUDE_DIRS)
else
    COMPILE_ARGS += $(ICARUS_DEFINES)
    COMPILE_ARGS += $(ICARUS_INCLUDE_DIRS)
endif

COMPILE_ARGS += $(COMPILE_ARGS_DEFINES)
ifeq ($(SIM), verilator)
    BUILD_ARGS += $(BUILD_ARGS_DEFINES)
endif

TOPLEVEL_LANG?=verilog
export TOPLEVEL_LANG

include $(shell cocotb-config --makefiles)/Makefile.sim
"""

    return mkfile
