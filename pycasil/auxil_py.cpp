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

#include <pycasil/pycasil.h>

#include <casil/auxil.h>

#include <optional>
#include <stdexcept>

namespace Auxil = casil::Auxil;

class PyAsyncIORunner
{
public:
    explicit PyAsyncIORunner(unsigned int pNumThreads = 1);
    ~PyAsyncIORunner();
    void enter();
    void exit(const std::optional<py::type>&, const std::optional<py::object>&, const std::optional<py::object>&);

private:
    const unsigned int numThreads;
};

void bindAuxil(py::module& pM)
{
    pM.def("uintSeqFromYAML", &Auxil::uintSeqFromYAML, "Parse a sequence of unsigned integers from YAML format.", py::arg("yamlString"));

    py::class_<PyAsyncIORunner>(pM, "AsyncIORunner", "Context manager to run IO context threads for ASIO functionality.")
            .def(py::init<unsigned int>(), "Constructor.", py::arg("numThreads") = 1)
            .def("__enter__", &PyAsyncIORunner::enter, "Start the numThreads IO context threads.", py::is_operator())
            .def("__exit__", &PyAsyncIORunner::exit, "Stop the IO context threads.", py::is_operator());
}

//Function definitions for AsyncIORunner wrapper class

PyAsyncIORunner::PyAsyncIORunner(const unsigned int pNumThreads) :
    numThreads(pNumThreads)
{
    if (numThreads == 0)
        throw std::runtime_error("Number of threads must be non-zero.");
}

PyAsyncIORunner::~PyAsyncIORunner()
{
    if (casil::ASIO::ioContextThreadsRunning())
        casil::ASIO::stopRunIOContext();
}

void PyAsyncIORunner::enter()
{
    if (!casil::ASIO::startRunIOContext(numThreads))
        throw std::runtime_error("Failed to start IO context threads.");
}

void PyAsyncIORunner::exit(const std::optional<py::type>&, const std::optional<py::object>&, const std::optional<py::object>&)
{
    if (casil::ASIO::ioContextThreadsRunning())
        casil::ASIO::stopRunIOContext();
}
