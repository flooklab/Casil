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
 * Defines the configuration wrapper structs InterfacesConf, DriversConf and RegistersConf, which replace the different layer
 * sections ("transfer_layer", "hw_drivers" and "registers") of the usual YAML configuration for Device in order to achieve a
 * compile-time configuration for \ref TemplateDeviceSpecialization "TemplateDevice", using the wrappers as its template arguments.
 *
 * Also defines the individual configuration wrapper structs InterfaceConf, DriverConf and RegisterConf,
 * which will in turn be passed as template arguments to the above wrappers in order to define the
 * configuration of the individual interfaces, drivers and registers within the different layers.
 *
 * See also \ref TemplateDeviceSpecialization "TemplateDevice".
 */
namespace TmplDev
{

/*!
 * \brief Implementation details for TmplDev.
 */
namespace TmplDevImpl
{
    /*!
     * \brief Common top-level base class for TmplDev component configuration wrappers.
     *
     * Declares a deleted constructor as the wrappers are supposed to work without instantiation.
     */
    struct ComponentConfBase { ComponentConfBase() = delete; };

    /*!
     * \brief Trivial differentiation of ComponentConfBase for only interface components.
     *
     * Used as base class for TmplDev::InterfaceConf.
     */
    struct InterfaceConfBase : public ComponentConfBase {};
    /*!
     * \brief Trivial differentiation of ComponentConfBase for only driver components.
     *
     * Used as base class for TmplDev::DriverConf.
     */
    struct DriverConfBase : public ComponentConfBase {};
    /*!
     * \brief Trivial differentiation of ComponentConfBase for only register components.
     *
     * Used as base class for TmplDev::RegisterConf.
     */
    struct RegisterConfBase : public ComponentConfBase {};

} // namespace TmplDevImpl

//

/*!
 * \brief Interface configuration wrapper for \ref TemplateDeviceSpecialization "TemplateDevice".
 *
 * This struct is intended to be derived from for every interface configuration needed. \p T must be the class type of the
 * interface to be configured. To properly define the interface's configuration (and make it usable by InterfacesConf)
 * the derived struct must declare the following members:
 *
 * - <tt>static constexpr char name[] = "instance_name_of_the_interface"</tt>
 * - <tt>static constexpr char conf[] = "interface: specific, yaml: configuration"</tt>
 *
 * \note Such a struct can be more easily defined via the \ref CASIL_DEFINE_INTERFACE macro from \ref templatedevicemacros.h.
 *
 * \tparam T Registered interface class implementing \ref Layers::TL::Interface "TL::Interface".
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
 * This struct is intended to be derived from for every driver configuration needed. \p T must be the class type of the
 * driver to be configured. To properly define the driver's configuration (and make it usable by DriversConf)
 * the derived struct must declare the following members:
 *
 * - <tt>static constexpr char name[] = "instance_name_of_the_driver"</tt>
 * - <tt>static constexpr char interface[] = "instance_name_of_the_used_interface"</tt>
 * - <tt>static constexpr char conf[] = "driver: specific, yaml: configuration"</tt>
 *
 * \note Such a struct can be more easily defined via the \ref CASIL_DEFINE_DRIVER macro from \ref templatedevicemacros.h.
 *
 * \tparam T Registered driver class implementing \ref Layers::HL::Driver "HL::Driver".
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
 * This struct is intended to be derived from for every register configuration needed. \p T must be the class type of the
 * register to be configured. To properly define the register's configuration (and make it usable by RegistersConf)
 * the derived struct must declare the following members:
 *
 * - <tt>static constexpr char name[] = "instance_name_of_the_register"</tt>
 * - <tt>static constexpr char driver[] = "instance_name_of_the_used_driver"</tt>
 * - <tt>static constexpr char conf[] = "register: specific, yaml: configuration"</tt>
 *
 * \note Such a struct can be more easily defined via the \ref CASIL_DEFINE_REGISTER macro from \ref templatedevicemacros.h.
 *
 * \tparam T Registered register class implementing \ref Layers::RL::Register "RL::Register".
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
    /*!
     * \brief Check if type is derived from a component configuration wrapper.
     *
     * Checks if \p T is derived from \p U, assuming that \p U is one of \ref casil::TmplDev::InterfaceConf "TmplDev::InterfaceConf",
     * \ref casil::TmplDev::DriverConf "TmplDev::DriverConf" or \ref casil::TmplDev::RegisterConf "TmplDev::RegisterConf".
     *
     * \tparam T Type to be checked.
     * \tparam U Component configuration wrapper struct that \p T should be derived from.
     */
    template<typename T, template<typename> typename U>
    concept DerivedFromConfStruct = requires { typename T::Type; } &&
                                    std::is_base_of_v<U<typename T::Type>, T> &&
                                    !std::is_same_v<U<typename T::Type>, T>;

    /*!
     * \brief Check if type properly implements a component configuration wrapper.
     *
     * Checks if \p T
     * - (a) is derived from \p U, assuming that \p U is one of \ref casil::TmplDev::InterfaceConf "TmplDev::InterfaceConf",
     *       \ref casil::TmplDev::DriverConf "TmplDev::DriverConf" or \ref casil::TmplDev::RegisterConf "TmplDev::RegisterConf", and
     * - (b) further defines two member variables that every component configuration wrapper must have:
     *   - `static constexpr char name[] = "name_of_interface";`
     *   - `static constexpr char conf[] = "possibly: empty, rest: of, yaml: configuration";`
     *
     * \tparam T Type to be checked.
     * \tparam U Component configuration wrapper struct that \p T should be derived from.
     */
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

} // namespace TmplDevImpl

//

/*!
 * \brief Check if type is a valid interface configuration wrapper.
 *
 * See \ref casil::TmplDev::InterfaceConf "InterfaceConf" for the requirements on \p T.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept ImplementsInterfaceConf = TmplDevImpl::ImplementsConfStruct<T, InterfaceConf>;

/*!
 * \brief Check if type is a valid driver configuration wrapper.
 *
 * See \ref casil::TmplDev::DriverConf "DriverConf" for the requirements on \p T.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept ImplementsDriverConf = TmplDevImpl::ImplementsConfStruct<T, DriverConf> && requires
{
    T::interface;
    requires Concepts::IsConstCharArr<decltype(T::interface)>;
    typename Concepts::TestConstexpr<T::interface[0]>;
};

/*!
 * \brief Check if type is a valid register configuration wrapper.
 *
 * See \ref casil::TmplDev::RegisterConf "RegisterConf" for the requirements on \p T.
 *
 * \tparam T Type to be checked.
 */
template<typename T>
concept ImplementsRegisterConf = TmplDevImpl::ImplementsConfStruct<T, RegisterConf> && requires
{
    T::driver;
    requires Concepts::IsConstCharArr<decltype(T::driver)>;
    typename Concepts::TestConstexpr<T::driver[0]>;
};

//

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of interface configurations.
 *
 * This struct, instantiated with \p Ts being configurations for individual interfaces, is meant to be passed as template argument
 * to \ref TemplateDeviceSpecialization "TemplateDevice" in order to compile-time-configure all interface components to be used.
 *
 * \tparam Ts Set of interface configurations (each implementing InterfaceConf).
 */
template<typename... Ts>
struct InterfacesConf
{
    static_assert((ImplementsInterfaceConf<Ts> && ...),
                  "Each interface must be specified by deriving from InterfaceConf and defining "
                  "'static constexpr char name[] = \"name_of_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of driver configurations.
 *
 * This struct, instantiated with \p Ts being configurations for individual drivers, is meant to be passed as template argument
 * to \ref TemplateDeviceSpecialization "TemplateDevice" in order to compile-time-configure all driver components to be used.
 *
 * \tparam Ts Set of driver configurations (each implementing DriverConf).
 */
template<typename... Ts>
struct DriversConf
{
    static_assert((ImplementsDriverConf<Ts> && ...),
                  "Each driver must be specified by deriving from DriverConf and defining "
                  "'static constexpr char name[] = \"name_of_driver\";' and "
                  "'static constexpr char interface[] = \"name_of_used_interface\";' and "
                  "'static constexpr char conf[] = \"possibly: empty, rest: of, yaml: configuration\";'.");
};

/*!
 * \brief \ref TemplateDeviceSpecialization "TemplateDevice" wrapper for a set of register configurations.
 *
 * This struct, instantiated with \p Ts being configurations for individual registers, is meant to be passed as template argument
 * to \ref TemplateDeviceSpecialization "TemplateDevice" in order to compile-time-configure all register components to be used.
 *
 * \tparam Ts Set of register configurations (each implementing RegisterConf).
 */
template<typename... Ts>
struct RegistersConf
{
    static_assert((ImplementsRegisterConf<Ts> && ...),
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
 */
template<typename InterfacesConfT, typename DriversConfT, typename RegistersConfT>
class TemplateDevice;

/*!
 * \copybrief TemplateDevice
 * \anchor TemplateDeviceSpecialization
 *
 * Wraps Device by enabling its configuration through the template arguments \p InterfaceConfTs, \p DriverConfTs and
 * \p RegisterConfTs, i.e. fixed at compile time, and by providing automatic component type conversion at compile time
 * for the return types of the component getters (see interface(), driver() and reg()), which enables for an uncomplicated
 * use of possibly component-specific functionality (unlike with Device, which requires manual casting in such cases).
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
     * Configures the Device by taking the compile-time configuration and generating an equivalent YAML configuration from it,
     * i.e. \p InterfaceConfTs / \p DriverConfTs / \p RegisterConfTs will be parsed as sequence elements for the
     * "transfer_layer" / "hw_drivers" / "registers" sections of a usual YAML configuration document for Device.
     *
     * See Device::Device(const boost::property_tree::ptree&) for details on the further processing.
     */
    TemplateDevice() :
        Device(generateConfig())
    {
    }
    /*!
     * \brief Default destructor.
     *
     * See Device::~Device().
     */
    ~TemplateDevice() override = default;

public:
    /*!
     * \brief Access one of the interface components from the transfer layer.
     *
     * Returns a reference to the interface component configured by \p T, which is equivalent to
     * Device::interface() being called with the configured instance name \c T::name, except that here
     * the return type is the specific interface type (\c T::Type ) instead of the base type \ref Layers::TL::Interface "TL::Interface".
     *
     * \tparam T An interface configuration out of \p InterfaceConfTs.
     * \return The interface component configured by \p T, casted to the specific interface type.
     */
    template<typename T>
        requires TmplDev::ImplementsInterfaceConf<T>
    typename T::Type& interface()
    {
        static_assert((std::is_same_v<T, InterfaceConfTs> || ...), "Device does not have the requested interface.");

        return dynamic_cast<typename T::Type&>(Device::interface(T::name));
    }
    /*!
     * \brief Access one of the driver components from the hardware layer.
     *
     * Returns a reference to the driver component configured by \p T, which is equivalent to
     * Device::driver() being called with the configured instance name \c T::name, except that here
     * the return type is the specific driver type (\c T::Type ) instead of the base type \ref Layers::HL::Driver "HL::Driver".
     *
     * \tparam T A driver configuration out of \p DriverConfTs.
     * \return The driver component configured by \p T, casted to the specific driver type.
     */
    template<typename T>
        requires TmplDev::ImplementsDriverConf<T>
    typename T::Type& driver()
    {
        static_assert((std::is_same_v<T, DriverConfTs> || ...), "Device does not have the requested driver.");

        return dynamic_cast<typename T::Type&>(Device::driver(T::name));
    }
    /*!
     * \brief Access one of the register components from the register layer.
     *
     * Returns a reference to the register component configured by \p T, which is equivalent to Device::reg() being
     * called with the configured instance name \c T::name, except that here the return type is the specific
     * register type (\c T::Type ) instead of the base type \ref Layers::RL::Register "RL::Register".
     *
     * \tparam T A register configuration out of \p RegisterConfTs.
     * \return The register component configured by \p T, casted to the specific register type.
     */
    template<typename T>
        requires TmplDev::ImplementsRegisterConf<T>
    typename T::Type& reg()
    {
        static_assert((std::is_same_v<T, RegisterConfTs> || ...), "Device does not have the requested register.");

        return dynamic_cast<typename T::Type&>(Device::reg(T::name));
    }

private:
    /*!
     * \brief Generate the device configuration tree from the template arguments.
     *
     * Generates the component configuration tree needed for initializing the Device base class.
     * This is done by generating YAML configuration code for the individual layers from the configuration wrappers passed
     * as class template parameters \p InterfaceConfTs, \p DriverConfTs and \p RegisterConfTs using addLayerElementsToYAML(),
     * inserting the resulting sequences into <tt>"{transfer_layer: #, hw_drivers: #, registers: #}"</tt>
     * (where the number signs are) and parsing the resulting YAML document by Auxil::propertyTreeFromYAML().
     *
     * \return %Device configuration tree as generated by Auxil::propertyTreeFromYAML() from a YAML configuration document.
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
     * Generates and adds the YAML code (a map) for the <tt>N</tt>-th component configuration out of \p ComponentConfTs
     * to the layer configuration sequence \p pYAMLLayerSeq, assuming that \p ComponentConfTs are from layer \p layer,
     * and recursively calls the function itself for the next component \p N+1 until the last element of \p ComponentConfTs.
     *
     * For the first and last components (i.e. if \p N equals \p 0 or <tt>sizeof...(ComponentConfTs)-1</tt>) opening and
     * closing brackets are added to \p pYAMLLayerSeq such that calling this function with <tt>N = 0</tt> results in a
     * full YAML sequence for layer component configurations \p ComponentConfTs for layer \p layer, as needed for Device.
     *
     * \tparam N Process the N-th component out of \p ComponentConfTs.
     * \tparam layer The layer of components configured by \p ComponentConfTs.
     * \tparam ComponentConfTs %Layer component configurations for \p layer, all implementing TmplDev::InterfaceConf,
     *         TmplDev::DriverConf or TmplDev::RegisterConf, depending on \p layer.
     * \param pYAMLLayerSeq Resulting YAML configuration sequence for layer \p layer.
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
