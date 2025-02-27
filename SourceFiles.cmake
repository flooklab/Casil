#[[
##################################################################################################
##
##  Copyright (C) 2024–2025 M. Frohne
##
##  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
##
##  Casil is free software: you can redistribute it and/or modify it
##  under the terms of the GNU Affero General Public License as published
##  by the Free Software Foundation, either version 3 of the License,
##  or (at your option) any later version.
##
##  Casil is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty
##  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##  See the GNU Affero General Public License for more details.
##
##  You should have received a copy of the GNU Affero General Public License
##  along with Casil. If not, see <https://www.gnu.org/licenses/>.
##
##################################################################################################
#]]

set(HEADER_FILE_NAMES
    asio.h
    auxil.h
    bytes.h
    concepts.h
    contextuallogger.h
    device.h
    env.h
    layerbase.h
    layerconfig.h
    layerfactory.h
    layerfactorymacros.h
    logger.h
    templatedevice.h
    templatedevicemacros.h
    version.h
    HL/directdriver.h
    HL/driver.h
    HL/muxeddriver.h
    HL/registerdriver.h
    HL/Direct/dummydriver.h
    HL/Direct/scpi.h
    HL/Direct/virtecho.h
    HL/Muxed/dummymuxeddriver.h
    HL/Muxed/gpio.h
    HL/Muxed/sitcpfifo.h
    RL/dummyregister.h
    RL/register.h
    TL/directinterface.h
    TL/interface.h
    TL/muxedinterface.h
    TL/CommonImpl/asiohelper.h
    TL/CommonImpl/serialportwrapper.h
    TL/CommonImpl/tcpsocketwrapper.h
    TL/CommonImpl/udpsocketwrapper.h
    TL/Direct/dummyinterface.h
    TL/Direct/serial.h
    TL/Direct/tcp.h
    TL/Direct/udp.h
    TL/Muxed/dummymuxedinterface.h
    TL/Muxed/sitcp.h
)

set(HEADER_FILE_NAMES_EXCLUDE_INSTALL
    TL/CommonImpl/asiohelper.h
    TL/CommonImpl/serialportwrapper.h
    TL/CommonImpl/tcpsocketwrapper.h
    TL/CommonImpl/udpsocketwrapper.h
)

set(SOURCE_FILE_NAMES
    asio
    assert
    auxil
    bytes
    contextuallogger
    device
    env
    layerbase
    layerconfig
    layerfactory
    logger
    version
    HL/directdriver
    HL/driver
    HL/muxeddriver
    HL/registerdriver
    HL/Direct/dummydriver
    HL/Direct/scpi
    HL/Direct/virtecho
    HL/Muxed/dummymuxeddriver
    HL/Muxed/gpio
    HL/Muxed/sitcpfifo
    RL/dummyregister
    RL/register
    TL/directinterface
    TL/interface
    TL/muxedinterface
    TL/CommonImpl/asiohelper
    TL/CommonImpl/serialportwrapper
    TL/CommonImpl/tcpsocketwrapper
    TL/CommonImpl/udpsocketwrapper
    TL/Direct/dummyinterface
    TL/Direct/serial
    TL/Direct/tcp
    TL/Direct/udp
    TL/Muxed/dummymuxedinterface
    TL/Muxed/sitcp
)

set(TESTS_FILE_NAMES
    tests.cpp
    datadirfixture.h
    core/test_asio/test_asio.cpp
    core/test_auxil/test_auxil.cpp
    core/test_bytes/test_bytes.cpp
    core/test_contextuallogger/test_contextuallogger.cpp
    core/test_contextuallogger/loggingdriver.cpp
    core/test_contextuallogger/loggingdriver.h
    core/test_device/test_device.cpp
    core/test_device/rtconfdriver.cpp
    core/test_device/rtconfdriver.h
    core/test_layerbase/test_layerbase.cpp
    core/test_layerbase/layertestclass.cpp
    core/test_layerbase/layertestclass.h
    core/test_layerbase/rtconftestclass.cpp
    core/test_layerbase/rtconftestclass.h
    core/test_layerconfig/test_layerconfig.cpp
    core/test_layerfactory/test_layerfactory.cpp
    core/test_layerpolymorphism/test_layerpolymorphism.cpp
    core/test_layerpolymorphism/testdrivermuxed.cpp
    core/test_layerpolymorphism/testdrivermuxed.h
    core/test_layerpolymorphism/testinterface.cpp
    core/test_layerpolymorphism/testinterface.h
    core/test_layerpolymorphism/wrongregister.cpp
    core/test_layerpolymorphism/wrongregister.h
    core/test_logger/test_logger.cpp
    core/test_templatedevice/test_templatedevice.cpp
    core/test_templatedevice/exampledevice.h
    core/test_templatedevice/testdriver.cpp
    core/test_templatedevice/testdriver.h
    core/test_templatedevicemacros/test_templatedevicemacros.cpp
    components/HL/test_gpio/test_gpio.cpp
    components/HL/test_registerdriver/test_registerdriver.cpp
    components/HL/test_registerdriver/fakeinterface.cpp
    components/HL/test_registerdriver/fakeinterface.h
    components/HL/test_registerdriver/invalidregdriver.cpp
    components/HL/test_registerdriver/invalidregdriver.h
    components/HL/test_registerdriver/testregdriver.cpp
    components/HL/test_registerdriver/testregdriver.h
    components/HL/test_scpi/test_scpi.cpp
    components/TL/test_tcp/test_tcp.cpp
    components/TL/test_udp/test_udp.cpp
)

set(SCPI_DEVICE_DESCRIPTION_FILE_NAMES
    agilent_33250a.yaml
    agilent_e3644a.yaml
    hp_81104a.yaml
    keithley_2000.yaml
    keithley_2001.yaml
    keithley_2400.yaml
    keithley_2410.yaml
    keithley_2450.yaml
    keithley_2460.yaml
    keithley_2602a.yaml
    keithley_2634b.yaml
    keithley_6517a.yaml
    scpi_sim_device.yaml
    tektronix_mso4034.yaml
    tektronix_mso4104b.yaml
    tti_ql355tp.yaml
)
