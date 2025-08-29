//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// \mainpage
///
/// Casil is a reimplementation of the data acquisition framework basil in C++.
/// It contains a C++ library to be used directly and also a Python binding to still
/// enable a script-based usage similar to basil itself. Casil does not contain any
/// firmware modules as one can simply use the original basil firmware modules. The
/// basic structure and usage of the library are aimed to be more or less compatible
/// to basil, but they are not strictly equivalent. If you use the C++ library directly
/// the general approach will be largely the same as with the Python binding or basil,
/// but some of the usage aspects might be quite different in detail and some very
/// "Pythonic" convenience features of basil might not be available at all.
///
/// Copyright (C) 2024–2025 M. Frohne and contributors\n
/// Copyright (C) 2011–2024 SiLab, Institute of Physics, University of Bonn
///
/// Casil is free software: you can redistribute it and/or modify it\n
/// under the terms of the GNU Affero General Public License as published\n
/// by the Free Software Foundation, either version 3 of the License,\n
/// or (at your option) any later version.
///
/// Casil is distributed in the hope that it will be useful,\n
/// but WITHOUT ANY WARRANTY; without even the implied warranty\n
/// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n
/// See the GNU Affero General Public License for more details.
///
/// You should have received a copy of the GNU Affero General Public License\n
/// along with Casil. If not, see <https://www.gnu.org/licenses/>.
///
/// \section example Example
/// Below you find a short example of how the Casil C++ library could be used.
///
/// \code
///
/// #include <casil/auxil.h>
/// #include <casil/bytes.h>
/// #include <casil/logger.h>
/// #include <casil/version.h>
///
/// #include <casil/templatedevice.h>
/// #include <casil/templatedevicemacros.h>
/// #include <casil/HL/Muxed/gpio.h>
/// #include <casil/HL/Muxed/sitcpfifo.h>
/// #include <casil/RL/standardregister.h>
/// #include <casil/TL/Muxed/sitcp.h>
///
/// #include <boost/dynamic_bitset.hpp>
///
/// #include <chrono>
/// #include <cstdint>
/// #include <string>
/// #include <thread>
/// #include <vector>
///
/// struct Intf : public casil::TmplDev::InterfaceConf<casil::TL::SiTCP>
/// {
///     static constexpr char name[] = "intf";
///     static constexpr char conf[] = "init: {ip: localhost, udp_port: 12345}";
/// };
///
/// struct GPIO : public casil::TmplDev::DriverConf<casil::HL::GPIO>
/// {
///     static constexpr char name[] = "GPIO";
///     static constexpr char interface[] = "intf";
///     static constexpr char conf[] = "base_addr: 0x0, size: 128";
/// };
///
/// struct FIFO : public casil::TmplDev::DriverConf<casil::HL::SiTCPFifo>
/// {
///     static constexpr char name[] = "FIFO";
///     static constexpr char interface[] = "intf";
///     static constexpr char conf[] = "base_addr: 0x0";    //"base_addr" is not used by SiTCPFifo
/// };
///
/// //Can use macros to simplify declaration of component configuration structs:
/// CASIL_DEFINE_REGISTER(casil::RL::StandardRegister, SomeRegister, "some_register", "GPIO", "size: 128, "
///                                                                                           "lsb_side_padding: False, "
///                                                                                           "fields: [ {name: TakeData, size: 1, offset: 5} ], "
///                                                                                           "init: { TakeData: 0 }")
///
/// //Above macro is equivalent to:
/// //
/// // struct SomeRegister : public casil::TmplDev::RegisterConf<casil::RL::StandardRegister>
/// // {
/// //     static constexpr char name[] = "some_register";
/// //     static constexpr char driver[] = "GPIO";
/// //     static constexpr char conf[] = "size: 128, "
/// //                                    "lsb_side_padding: False, "
/// //                                    "fields: [ {name: TakeData, size: 1, offset: 5} ], "
/// //                                    "init: { TakeData: 0 }";
/// // };
///
/// typedef casil::TemplateDevice<
///                 casil::TmplDev::InterfacesConf<Intf>,
///                 casil::TmplDev::DriversConf<GPIO,
///                                             FIFO>,
///                 casil::TmplDev::RegistersConf<SomeRegister>
///             > ExampleDevice;
///
/// int main()
/// {
///     using casil::Logger;
///     using LogLevel = Logger::LogLevel;
///     Logger::setLogLevel(LogLevel::Info);
///     Logger::addOutputCout();
///
///     Logger::logInfo("Casil version: " + casil::Version::toString());
///
///     ExampleDevice exampleDev;
///
///     std::vector<std::uint32_t> fifoData;
///
///     {
///         casil::Auxil::AsyncIORunner<1> ioRunner;
///         (void)ioRunner;
///
///         exampleDev.init();
///
///         auto& gpioReg = exampleDev.reg<SomeRegister>();
///
///         //Start taking data
///         gpioReg["TakeData"][0] = true;
///         gpioReg.write();
///
///         std::this_thread::sleep_for(std::chrono::seconds{3});
///
///         //Stop taking data
///         gpioReg["TakeData"][0] = false;
///         gpioReg.write();
///
///         std::this_thread::sleep_for(std::chrono::milliseconds{200});
///
///         //Read data
///         fifoData = exampleDev.driver<FIFO>().getFifoData();
///
///         exampleDev.close();
///     }
///
///     Logger::logInfo("Received FIFO data: " + casil::Bytes::formatUInt32Vec(fifoData));
///
///     Logger::logSuccess("Done.");
///
///     return 0;
/// }
///
/// \endcode
///
//////////////////////////////////////////////////////////////////////////////////////////////////
