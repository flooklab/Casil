/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024 M. Frohne
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

#ifndef CASIL_CONCEPTS_CONCEPTS_H
#define CASIL_CONCEPTS_CONCEPTS_H

#include <casil/layerconfig.h>
#include <casil/HL/driver.h>

#include <string>
#include <type_traits>

namespace casil
{

namespace TL { class Interface; }
namespace RL { class Register; }

namespace Concepts
{

template<char c>
using TestConstexpr = void;

template<typename T>
concept IsConstCharArr = requires
{
    requires std::is_array_v<T>;    //Is array type
    requires std::rank_v<T> == 1;   //Is 1-dim. array
    requires std::is_same_v<std::remove_extent_t<const char[]>, std::remove_extent_t<T>>;   //Type corresponds to const char[]
};

template<typename T>
concept HasRegisteredTypeName = requires    //Must have 'static constexpr char typeName[]' member
{
    T::typeName;                                    //Member exists
    requires IsConstCharArr<decltype(T::typeName)>; //Type corresponds to const char[]
    typename TestConstexpr<T::typeName[0]>;         //Is usable as constexpr
};

template<typename T>
concept HasInterfaceBaseType = requires     //Must declare typedef for abstract class derived from TL::Interface
{
    typename T::InterfaceBaseType;
    requires (std::is_base_of_v<TL::Interface, typename T::InterfaceBaseType> &&    //Derived from Interface
             !std::is_same_v<TL::Interface, typename T::InterfaceBaseType>) &&      //Not Interface itself
             std::is_abstract_v<typename T::InterfaceBaseType>;                     //Abstract, i.e. no actual interface
};

template<typename T>
concept IsInterface = std::is_base_of_v<TL::Interface, T> &&
                      std::is_constructible_v<T, std::string, LayerConfig>;

template<typename T>
concept IsDriver = std::is_base_of_v<HL::Driver, T> &&
                   HasInterfaceBaseType<T> &&
                   std::is_constructible_v<T, std::string, typename T::InterfaceBaseType&, LayerConfig>;

template<typename T>
concept IsRegister = std::is_base_of_v<RL::Register, T> &&
                     std::is_constructible_v<T, std::string, HL::Driver&, LayerConfig>;

} // namespace Concepts

} // namespace casil

#endif // CASIL_CONCEPTS_CONCEPTS_H
