/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024–2025 M. Frohne
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
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

#include <boost/asio/detail/config.hpp>

#include <climits>
#include <cstdint>
#include <type_traits>

//Require equivalent use of std::uint8_t and (unsigned) char as 8 bit wide bytes (see https://stackoverflow.com/a/16261758/13684381)
static_assert(CHAR_BIT == 8, "Number of bits in a byte must be 8.");
static_assert(std::is_same_v<std::uint8_t, char> || std::is_same_v<std::uint8_t, unsigned char>,
              "Type std::uint8_t must be either char or unsigned char.");

//Availability of the serial port implementation is platform dependent
static_assert(BOOST_ASIO_HAS_SERIAL_PORT, "No serial port implementation available on the current platform.");
