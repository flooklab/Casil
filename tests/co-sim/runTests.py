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

import importlib.util
import os
import subprocess
import sys

#Determine all test files

testFiles = []
for directory, _, files in os.walk(os.path.join(os.path.dirname(__file__), "tests")):
    testFiles += [os.path.join(directory, filename) for filename in files if (filename.startswith("test_") and filename.endswith(".py"))]

#Add path to 'PyCasilSim' simulation tools to subprocesses' environment

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "sim-tools"))

modEnv = os.environ.copy()
modEnv['PYTHONPATH'] = os.pathsep.join(sys.path)

#Run all tests, using 'pytest' if available

if not os.getenv("NO_PYTEST") and importlib.util.find_spec("pytest") is not None:
    if os.getenv("SIM") == "verilator":
        subprocess.run([sys.executable, "-m", "pytest", "-m", "verilator"] + testFiles, env=modEnv)
    else:
        subprocess.run([sys.executable, "-m", "pytest"] + testFiles, env=modEnv)
else:
    for testFile in testFiles:
        subprocess.run([sys.executable, testFile], env=modEnv)
