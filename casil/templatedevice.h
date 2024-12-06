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

#ifndef CASIL_TEMPLATEDEVICE_H
#define CASIL_TEMPLATEDEVICE_H

#include <casil/device.h>

#include <casil/auxil.h>
#include <casil/concepts.h>
#include <casil/layerbase.h>

#include <boost/property_tree/ptree_fwd.hpp>

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

namespace casil
{

namespace TmplDevImpl
{
    struct TmplDevComponentBase { TmplDevComponentBase() = delete; };

    struct TmplDevInterfaceBase : public TmplDevComponentBase {};
    struct TmplDevDriverBase : public TmplDevComponentBase {};
    struct TmplDevRegisterBase : public TmplDevComponentBase {};

} // namespace TmplDevImpl

//

/*!
 * \brief Interface configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * \todo Detailed doc
 *
 * \tparam T %Interface class implementing TL::Interface.
 */
template<typename T>
struct TmplDevInterface : public TmplDevImpl::TmplDevInterfaceBase
{
    static_assert(Concepts::IsInterface<T>, "Type must be an interface.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;
};

/*!
 * \brief Driver configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * \todo Detailed doc
 *
 * \tparam T %Driver class implementing HL::Driver.
 */
template<typename T>
struct TmplDevDriver : public TmplDevImpl::TmplDevDriverBase
{
    static_assert(Concepts::IsDriver<T>, "Type must be a driver.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;
};

/*!
 * \brief Register configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * \todo Detailed doc
 *
 * \tparam T %Register class implementing RL::Register.
 */
template<typename T>
struct TmplDevRegister : public TmplDevImpl::TmplDevRegisterBase
{
    static_assert(Concepts::IsRegister<T>, "Type must be a register.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;
};

//

namespace TmplDevImpl
{
    template<typename T, template <typename> typename U>
    concept DerivedFromTmplDevStruct = requires { typename T::Type; } &&
                                       std::is_base_of_v<U<typename T::Type>, T> &&
                                       !std::is_same_v<U<typename T::Type>, T>;

    template<typename T, template <typename> typename U>
    concept ImplementsTmplDevStruct = DerivedFromTmplDevStruct<T, U> && requires
    {
        T::name;
        T::conf;
        requires Concepts::IsConstCharArr<decltype(T::name)>;
        requires Concepts::IsConstCharArr<decltype(T::conf)>;
        typename Concepts::TestConstexpr<T::name[0]>;
        typename Concepts::TestConstexpr<T::conf[0]>;
    };

    template<typename T>
    concept ImplementsTmplDevInterface = ImplementsTmplDevStruct<T, TmplDevInterface>;

    template<typename T>
    concept ImplementsTmplDevDriver = ImplementsTmplDevStruct<T, TmplDevDriver> && requires
    {
        T::interface;
        requires Concepts::IsConstCharArr<decltype(T::interface)>;
        typename Concepts::TestConstexpr<T::interface[0]>;
    };

    template<typename T>
    concept ImplementsTmplDevRegister = ImplementsTmplDevStruct<T, TmplDevRegister> && requires
    {
        T::driver;
        requires Concepts::IsConstCharArr<decltype(T::driver)>;
        typename Concepts::TestConstexpr<T::driver[0]>;
    };

} // namespace TmplDevImpl

//

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of interface configurations.
 *
 * \todo Detailed doc
 *
 * \tparam Ts Set of interface configurations (each implementing TmplDevInterface).
 */
template<typename... Ts>
struct TmplDevInterfaces
{
    static_assert((TmplDevImpl::ImplementsTmplDevInterface<Ts> && ...),
                  "Each interface must be specified by deriving from TmplDevInterface and defining "
                  "'static constexpr char name[] = \"name_of_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of driver configurations.
 *
 * \todo Detailed doc
 *
 * \tparam Ts Set of driver configurations (each implementing TmplDevDriver).
 */
template<typename... Ts>
struct TmplDevDrivers
{
    static_assert((TmplDevImpl::ImplementsTmplDevDriver<Ts> && ...),
                  "Each driver must be specified by deriving from TmplDevDriver and defining "
                  "'static constexpr char name[] = \"name_of_driver\";' and "
                  "'static constexpr char interface[] = \"name_of_used_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of register configurations.
 *
 * \todo Detailed doc
 *
 * \tparam Ts Set of register configurations (each implementing TmplDevRegister).
 */
template<typename... Ts>
struct TmplDevRegisters
{
    static_assert((TmplDevImpl::ImplementsTmplDevRegister<Ts> && ...),
                  "Each register must be specified by deriving from TmplDevRegister and defining "
                  "'static constexpr char name[] = \"name_of_register\";' and "
                  "'static constexpr char driver[] = \"name_of_used_hw_driver\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

//

/*!
 * \brief Type-safe wrapper for the "plain" Device class.
 *
 * See \ref TemplateDeviceSpecialization "TemplateDevice<TmplDevInterfaces<TmplDevInterfaceTs...>, <!--
 *                                                    -->TmplDevDrivers<TmplDevDriverTs...>, TmplDevRegisters<TmplDevRegisterTs...>>"
 */
template<typename TmplDevInterfacesT, typename TmplDevDriversT, typename TmplDevRegistersT>
class TemplateDevice;

/*!
 * \copybrief TemplateDevice
 * \anchor TemplateDeviceSpecialization
 *
 * \todo Detailed doc
 *
 * \tparam TmplDevInterfaces Wrapper for a set of interface configurations \p TmplDevInterfaceTs.
 * \tparam TmplDevInterfaceTs Set of interface configurations (each implementing TmplDevInterface).
 * \tparam TmplDevDrivers Wrapper for a set of driver configurations \p TmplDevDriverTs.
 * \tparam TmplDevDriverTs Set of driver configurations (each implementing TmplDevDriver).
 * \tparam TmplDevRegisters Wrapper for a set of register configurations.
 * \tparam TmplDevRegisterTs Set of register configurations (each implementing TmplDevRegister).
 */
template<typename... TmplDevInterfaceTs, typename... TmplDevDriverTs, typename... TmplDevRegisterTs>
class TemplateDevice<TmplDevInterfaces<TmplDevInterfaceTs...>,
                     TmplDevDrivers<TmplDevDriverTs...>,
                     TmplDevRegisters<TmplDevRegisterTs...>> : public Device
{
public:
    TemplateDevice() :
        Device(generateConfig())
    {
    }
    ~TemplateDevice() override = default;

public:
    template<typename T>
    typename T::Type& interface()
    {
        static_assert(std::is_base_of_v<TL::Interface, typename T::Type>, "Requested type is not an interface.");
        static_assert((std::is_same_v<T, TmplDevInterfaceTs> || ...), "Device does not have the requested interface.");

        return dynamic_cast<typename T::Type&>(Device::interface(T::name));
    }
    template<typename T>
    typename T::Type& driver()
    {
        static_assert(std::is_base_of_v<HL::Driver, typename T::Type>, "Requested type is not a driver.");
        static_assert((std::is_same_v<T, TmplDevDriverTs> || ...), "Device does not have the requested driver.");

        return dynamic_cast<typename T::Type&>(Device::driver(T::name));
    }
    template<typename T>
    typename T::Type& reg()
    {
        static_assert(std::is_base_of_v<RL::Register, typename T::Type>, "Requested type is not a register.");
        static_assert((std::is_same_v<T, TmplDevRegisterTs> || ...), "Device does not have the requested register.");

        return dynamic_cast<typename T::Type&>(Device::reg(T::name));
    }

private:
    static boost::property_tree::ptree generateConfig()
    {
        std::string yamlIntfSeq;
        std::string yamlDrvSeq;
        std::string yamlRegSeq;

        if constexpr (sizeof...(TmplDevInterfaceTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::TransferLayer, TmplDevInterfaceTs...>(yamlIntfSeq);
        if constexpr (sizeof...(TmplDevDriverTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::HardwareLayer, TmplDevDriverTs...>(yamlDrvSeq);
        if constexpr (sizeof...(TmplDevRegisterTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::RegisterLayer, TmplDevRegisterTs...>(yamlRegSeq);

        const std::string yamlString = "{transfer_layer: " + yamlIntfSeq + ", " +
                                       "hw_drivers: " + yamlDrvSeq + ", " +
                                       "registers: " + yamlRegSeq + "}";

        return Auxil::propertyTreeFromYAML(yamlString);
    }
    //
    template<std::size_t N, LayerBase::Layer layer, typename... TmplDevComponentTs>
    static void addLayerElementsToYAML(std::string& pYAMLLayerSeq)
    {
        using CurrentElementT = std::tuple_element_t<N, std::tuple<TmplDevComponentTs...>>;

        if constexpr (N == 0)
            pYAMLLayerSeq = "[";
        else
            pYAMLLayerSeq += ", ";

        std::string tElementMap = std::string("{name: ") + CurrentElementT::name + ", " + "type: " + CurrentElementT::Type::typeName;

        if constexpr (layer == LayerBase::Layer::HardwareLayer)
            tElementMap += std::string(", interface: ") + CurrentElementT::interface;
        else if constexpr (layer == LayerBase::Layer::RegisterLayer)
            tElementMap += std::string(", hw_driver: ") + CurrentElementT::driver;

        tElementMap += std::string(", ") + CurrentElementT::conf + "}";

        pYAMLLayerSeq += tElementMap;

        if constexpr (N < sizeof...(TmplDevComponentTs)-1)
            addLayerElementsToYAML<N+1, layer, TmplDevComponentTs...>(pYAMLLayerSeq);
        else
            pYAMLLayerSeq += "]";
    }
};

} // namespace casil

#endif // CASIL_TEMPLATEDEVICE_H
