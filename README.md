# Casil

Casil is an attempt at a reimplementation of the data acquisition framework [basil](https://github.com/SiLab-Bonn/basil) in C++.
It contains a C++ library to be used directly and also a Python binding to still enable a script-based usage similar to basil itself.
Casil does not contain any firmware modules as one can simply use the original basil firmware modules.  

The basic structure and usage of the library are aimed to be more or less compatible to basil, but they are not
strictly equivalent. If you use the C++ library directly, the general approach will be largely the same as with basil,
but some of the usage aspects might be quite different in detail and some very "Pythonic" convenience features of basil
might not be available at all. The usage of the Python binding ("PyCasil") will be rather similar to basil. If you wanted
to use PyCasil in place of basil, though, this would possibly require small changes to your YAML configuration and
certainly require bigger (but not huge) changes to your program code. Note, however, that Casil is currently not
really usable because it is still nearly incomplete and not even tested with hardware or simulation yet.  
Please also see the ["disclaimer"](#user-content-disclaimer) below.  

For license information please see the [License Information](#user-content-license-information) section at the bottom.

## Overview

### Disclaimer

Please note the following:  

For anyone wondering what the point of this project is: you should not take it
too seriously at the moment as I mainly created it for the following reasons:
- For myself to improve on or learn: C++, CMake, writing unit tests, creating Python bindings,
  code documentation, general software engineering/distribution
- To have something like this specialized for my personal use
- Code quality and documentation of basil could be even further improved,
  hence this was maybe worth a try anyway

I also currently do not see that anyone would really want or need something like Casil (if you **do**, let me know...).
But, as this project actually turned out to be relatively solid/functioning already, I thought I would share it anyway.
Nevertheless, as I wrote above, Casil is still nearly incomplete. For it to become anything serious that could be used
for an actual project one would need to port a whole lot of individual drivers and other parts from basil to Casil.
The whole `cocotb` simulation interface and accompanying simulation-driven unit tests would have to be ported too
at some point. I do not see this happening anytime soon (unless maybe someone would want to help out).

## Build

Building the project can be configured with [CMake](https://cmake.org/) (>= v3.23).  

You will need a `C++20` compatible compiler, probably at least `GCC 13` or equivalent.  

Required additional libraries/packages:
- [Python](https://www.python.org/) (>= v3.10)
- [Boost](https://www.boost.org/) (>= v1.70; libraries: Algorithm, Asio, DynamicBitset, Predef, PropertyTree, System, Test (optional for unit tests))
- [pybind11](https://github.com/pybind/pybind11) (>= v2.13.4; contained as `Git` submodule)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) (>= v0.8.0; contained as `Git` submodule)

Optional packages:
- Building the documentation requires [Doxygen](https://github.com/doxygen/doxygen) (>= v1.12)

## License Information

Copyright (C) 2024–2025 M. Frohne and contributors  
Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn  

Casil is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.  

Casil is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Affero General Public License for more details.  

You should have received a [copy](LICENSE) of the GNU Affero General Public License
along with Casil. If not, see <https://www.gnu.org/licenses/>.

### Derivative Work

Parts of Casil have been more or less directly adapted from the original
[basil](https://github.com/SiLab-Bonn/basil) software,
which is covered by the following license:  

```
Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 *  Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### External Libraries

Casil uses (i.e. links to) the following external libraries:

- [Boost](https://www.boost.org/) (Algorithm, Asio, DynamicBitset, Predef, PropertyTree, System, Test):  

```
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
```

- [pybind11](https://github.com/pybind/pybind11):  

```
Copyright (c) 2015–2024 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

- [yaml-cpp](https://github.com/jbeder/yaml-cpp):  

```
Copyright (c) 2008–2023 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
