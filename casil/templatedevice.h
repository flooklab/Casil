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

/*!
 * \brief Compile-time configuration of \ref TemplateDeviceSpecialization "TemplateDevice" layer components.
 *
 * \todo Detailed doc
 */
namespace TmplDev
{

/*!
 * \brief Implementation details for TmplDev.
 */
namespace TmplDevImpl
{
    struct ComponentConfBase { ComponentConfBase() = delete; };

    struct InterfaceConfBase : public ComponentConfBase {};
    struct DriverConfBase : public ComponentConfBase {};
    struct RegisterConfBase : public ComponentConfBase {};

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
struct InterfaceConf : public TmplDevImpl::InterfaceConfBase
{
    static_assert(Concepts::IsInterface<T>, "Type must be an interface.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;     ///< The wrapped interface type.
};

/*!
 * \brief Driver configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * \todo Detailed doc
 *
 * \tparam T %Driver class implementing HL::Driver.
 */
template<typename T>
struct DriverConf : public TmplDevImpl::DriverConfBase
{
    static_assert(Concepts::IsDriver<T>, "Type must be a driver.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;     ///< The wrapped driver type.
};

/*!
 * \brief Register configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * \todo Detailed doc
 *
 * \tparam T %Register class implementing RL::Register.
 */
template<typename T>
struct RegisterConf : public TmplDevImpl::RegisterConfBase
{
    static_assert(Concepts::IsRegister<T>, "Type must be a register.");
    static_assert(Concepts::HasRegisteredTypeName<T>, "Type must be registered to the factory.");

    typedef T Type;     ///< The wrapped register type.
};

//

namespace TmplDevImpl
{
    template<typename T, template<typename> typename U>
    concept DerivedFromConfStruct = requires { typename T::Type; } &&
                                    std::is_base_of_v<U<typename T::Type>, T> &&
                                    !std::is_same_v<U<typename T::Type>, T>;

    template<typename T, template<typename> typename U>
    concept ImplementsConfStruct = DerivedFromConfStruct<T, U> && requires
    {
        T::name;
        T::conf;
        requires Concepts::IsConstCharArr<decltype(T::name)>;
        requires Concepts::IsConstCharArr<decltype(T::conf)>;
        typename Concepts::TestConstexpr<T::name[0]>;
        typename Concepts::TestConstexpr<T::conf[0]>;
    };

    template<typename T>
    concept ImplementsInterfaceConf = ImplementsConfStruct<T, InterfaceConf>;

    template<typename T>
    concept ImplementsDriverConf = ImplementsConfStruct<T, DriverConf> && requires
    {
        T::interface;
        requires Concepts::IsConstCharArr<decltype(T::interface)>;
        typename Concepts::TestConstexpr<T::interface[0]>;
    };

    template<typename T>
    concept ImplementsRegisterConf = ImplementsConfStruct<T, RegisterConf> && requires
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
 * \tparam Ts Set of interface configurations (each implementing InterfaceConf).
 */
template<typename... Ts>
struct InterfacesConf
{
    static_assert((TmplDevImpl::ImplementsInterfaceConf<Ts> && ...),
                  "Each interface must be specified by deriving from InterfaceConf and defining "
                  "'static constexpr char name[] = \"name_of_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of driver configurations.
 *
 * \todo Detailed doc
 *
 * \tparam Ts Set of driver configurations (each implementing DriverConf).
 */
template<typename... Ts>
struct DriversConf
{
    static_assert((TmplDevImpl::ImplementsDriverConf<Ts> && ...),
                  "Each driver must be specified by deriving from DriverConf and defining "
                  "'static constexpr char name[] = \"name_of_driver\";' and "
                  "'static constexpr char interface[] = \"name_of_used_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of register configurations.
 *
 * \todo Detailed doc
 *
 * \tparam Ts Set of register configurations (each implementing RegisterConf).
 */
template<typename... Ts>
struct RegistersConf
{
    static_assert((TmplDevImpl::ImplementsRegisterConf<Ts> && ...),
                  "Each register must be specified by deriving from RegisterConf and defining "
                  "'static constexpr char name[] = \"name_of_register\";' and "
                  "'static constexpr char driver[] = \"name_of_used_hw_driver\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

} // namespace TmplDev

//

/*!
 * \brief Type-safe wrapper for the "plain" Device class.
 *
 * See \ref TemplateDeviceSpecialization "TemplateDevice<TmplDev::InterfacesConf<InterfaceConfTs...>, <!--
 *                                                      -->TmplDev::DriversConf<DriverConfTs...>, TmplDev::RegistersConf<RegisterConfTs...>>"
 *
 * \todo Describe or remove tparams here.
 *
 * \tparam InterfacesConfT
 * \tparam DriversConfT
 * \tparam RegistersConfT
 */
template<typename InterfacesConfT, typename DriversConfT, typename RegistersConfT>
class TemplateDevice;

/*!
 * \copybrief TemplateDevice
 * \anchor TemplateDeviceSpecialization
 *
 * \todo Detailed doc
 *
 * \tparam TmplDev::InterfacesConf Wrapper for a set of interface configurations \p InterfaceConfTs.
 * \tparam InterfaceConfTs Set of interface configurations (each implementing TmplDev::InterfaceConf).
 * \tparam TmplDev::DriversConf Wrapper for a set of driver configurations \p DriverConfTs.
 * \tparam DriverConfTs Set of driver configurations (each implementing TmplDev::DriverConf).
 * \tparam TmplDev::RegistersConf Wrapper for a set of register configurations \p RegisterConfTs.
 * \tparam RegisterConfTs Set of register configurations (each implementing TmplDev::RegisterConf).
 */
template<typename... InterfaceConfTs, typename... DriverConfTs, typename... RegisterConfTs>
class TemplateDevice<TmplDev::InterfacesConf<InterfaceConfTs...>,
                     TmplDev::DriversConf<DriverConfTs...>,
                     TmplDev::RegistersConf<RegisterConfTs...>> : public Device
{
public:
    /*!
     * \brief Constructor.
     *
     * \todo Detailed doc
     */
    TemplateDevice() :
        Device(generateConfig())
    {
    }
    /*!
     * \brief Default destructor.
     */
    ~TemplateDevice() override = default;

public:
    /*!
     * \brief Access one of the interface components from the transfer layer.
     *
     * \todo Detailed doc
     *
     * \tparam T
     * \return
     */
    template<typename T>
    typename T::Type& interface()
    {
        static_assert(std::is_base_of_v<TL::Interface, typename T::Type>, "Requested type is not an interface.");
        static_assert((std::is_same_v<T, InterfaceConfTs> || ...), "Device does not have the requested interface.");

        return dynamic_cast<typename T::Type&>(Device::interface(T::name));
    }
    /*!
     * \brief Access one of the driver components from the hardware layer.
     *
     * \todo Detailed doc
     *
     * \tparam T
     * \return
     */
    template<typename T>
    typename T::Type& driver()
    {
        static_assert(std::is_base_of_v<HL::Driver, typename T::Type>, "Requested type is not a driver.");
        static_assert((std::is_same_v<T, DriverConfTs> || ...), "Device does not have the requested driver.");

        return dynamic_cast<typename T::Type&>(Device::driver(T::name));
    }
    /*!
     * \brief Access one of the register components from the register layer.
     *
     * \todo Detailed doc
     *
     * \tparam T
     * \return
     */
    template<typename T>
    typename T::Type& reg()
    {
        static_assert(std::is_base_of_v<RL::Register, typename T::Type>, "Requested type is not a register.");
        static_assert((std::is_same_v<T, RegisterConfTs> || ...), "Device does not have the requested register.");

        return dynamic_cast<typename T::Type&>(Device::reg(T::name));
    }

private:
    /*!
     * \brief Generate the device configuration tree from the template arguments.
     *
     * \todo Detailed doc
     *
     * \return
     */
    static boost::property_tree::ptree generateConfig()
    {
        std::string yamlIntfSeq;
        std::string yamlDrvSeq;
        std::string yamlRegSeq;

        if constexpr (sizeof...(InterfaceConfTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::TransferLayer, InterfaceConfTs...>(yamlIntfSeq);
        if constexpr (sizeof...(DriverConfTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::HardwareLayer, DriverConfTs...>(yamlDrvSeq);
        if constexpr (sizeof...(RegisterConfTs) > 0)
            addLayerElementsToYAML<0, LayerBase::Layer::RegisterLayer, RegisterConfTs...>(yamlRegSeq);

        const std::string yamlString = "{transfer_layer: " + yamlIntfSeq + ", " +
                                       "hw_drivers: " + yamlDrvSeq + ", " +
                                       "registers: " + yamlRegSeq + "}";

        return Auxil::propertyTreeFromYAML(yamlString);
    }
    //
    /*!
     * \brief Recursively generate the YAML sequence for the components of a certain layer.
     *
     * \todo Detailed doc
     *
     * \tparam N
     * \tparam layer
     * \tparam ComponentConfTs
     * \param pYAMLLayerSeq
     */
    template<std::size_t N, LayerBase::Layer layer, typename... ComponentConfTs>
    static void addLayerElementsToYAML(std::string& pYAMLLayerSeq)
    {
        using CurrentElementT = std::tuple_element_t<N, std::tuple<ComponentConfTs...>>;

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

        if constexpr (N < sizeof...(ComponentConfTs)-1)
            addLayerElementsToYAML<N+1, layer, ComponentConfTs...>(pYAMLLayerSeq);
        else
            pYAMLLayerSeq += "]";
    }
};

} // namespace casil

#endif // CASIL_TEMPLATEDEVICE_H
