#[[
##################################################################################################
##
##  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
##  Copyright (C) 2024 M. Frohne
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

cmake_minimum_required(VERSION 3.23)

if(NOT DEFINED CACHE{CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

#Tweak CMake defaults for compiler flags

if(NOT MSVC)
    set(DEFAULT_FLAGS "")
    set(DEFAULT_FLAGS_DEBUG "-g -Og")
    set(DEFAULT_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    set(DEFAULT_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(DEFAULT_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
else()
    #Those MSVC flags are probably not tested
    set(DEFAULT_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
    set(DEFAULT_FLAGS_MINSIZEREL "/MD /O1 /Ob1 /DNDEBUG")
    set(DEFAULT_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG")
    set(DEFAULT_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Ob1 /DNDEBUG")
endif()

if(NOT DEFINED CACHE{CMAKE_CXX_FLAGS})
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS "${DEFAULT_FLAGS}" CACHE STRING "Flags used by the CXX compiler during all build types." FORCE)
    endif()
endif()
if(NOT DEFINED CACHE{CMAKE_CXX_FLAGS_DEBUG})
    set(CMAKE_CXX_FLAGS_DEBUG "${DEFAULT_FLAGS_DEBUG}" CACHE STRING "Flags used by the CXX compiler during DEBUG builds." FORCE)
endif()
if(NOT DEFINED CACHE{CMAKE_CXX_FLAGS_MINSIZEREL})
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${DEFAULT_FLAGS_MINSIZEREL}" CACHE STRING "Flags used by the CXX compiler during MINSIZEREL builds." FORCE)
endif()
if(NOT DEFINED CACHE{CMAKE_CXX_FLAGS_RELEASE})
    set(CMAKE_CXX_FLAGS_RELEASE "${DEFAULT_FLAGS_RELEASE}" CACHE STRING "Flags used by the CXX compiler during RELEASE builds." FORCE)
endif()
if(NOT DEFINED CACHE{CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${DEFAULT_FLAGS_RELWITHDEBINFO}" CACHE STRING "Flags used by the CXX compiler during RELWITHDEBINFO builds." FORCE)
endif()

project(Casil VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

#Configure options

if(NOT MSVC)
    string(CONCAT TMP_CXX_FLAGS "-Wall;-Wextra;-Wshadow;-pedantic-errors;"
                                "-pipe;-march=x86-64;-mtune=generic;-Wp,-D_FORTIFY_SOURCE=3;-Wp,-D_GLIBCXX_ASSERTIONS;"
                                "-fno-plt;-fstack-protector-strong;-fstack-clash-protection;-fcf-protection=full;-flto=auto")
    set(CASIL_CXX_FLAGS "${TMP_CXX_FLAGS}" CACHE STRING "Additional compiler flags.")
    set(CASIL_LINK_FLAGS "-Wl,--sort-common;-Wl,--as-needed;-Wl,-z,defs;-Wl,-z,relro;-Wl,-z,now;-Wl,-z,pack-relative-relocs"
        CACHE STRING "Additional linker flags.")
    set(CASIL_LINK_FLAGS_DEBUG "" CACHE STRING "Additional linker flags to add for DEBUG builds.")
    set(CASIL_LINK_FLAGS_RELEASE "-Wl,-O1" CACHE STRING "Additional linker flags to add for non-DEBUG builds.")
else()
    #Those MSVC flags are probably not tested
    string(CONCAT TMP_CXX_FLAGS "/W4;/permissive-;"
                                "/GL;/GS;/guard:cf;/guard:ehcont")
    set(CASIL_CXX_FLAGS "${TMP_CXX_FLAGS}" CACHE STRING "Additional compiler flags.")
    set(CASIL_LINK_FLAGS "/CETCOMPAT;/DYNAMICBASE;/OPT;/LTCG" CACHE STRING "Additional linker flags.")
    set(CASIL_LINK_FLAGS_DEBUG "/DEBUG" CACHE STRING "Additional linker flags to add for DEBUG builds.")
    set(CASIL_LINK_FLAGS_RELEASE "" CACHE STRING "Additional linker flags to add for non-DEBUG builds.")
endif()

if(NOT MSVC)
    set(CASIL_ENABLE_GLIBCXX_DEBUG_MODE OFF CACHE BOOL "Add compiler flags to enable the debug mode of libstdc++.")
endif()

set(CASIL_BUILD_STATIC ON CACHE BOOL "Build static Casil library.")
set(CASIL_BUILD_SHARED ON CACHE BOOL "Build shared Casil library.")
set(CASIL_BUILD_BINDING ON CACHE BOOL "Build PyCasil Python binding.")
set(CASIL_BUILD_EXAMPLE ON CACHE BOOL "Build example executable.")
set(CASIL_BUILD_TESTS ON CACHE BOOL "Build Casil unit tests.")

if((NOT CASIL_BUILD_STATIC) AND (NOT CASIL_BUILD_SHARED))
    message(FATAL_ERROR "Must build at least one version of the library (shared/static).")
endif()

if(CASIL_BUILD_BINDING AND (NOT CASIL_BUILD_STATIC))
    message(FATAL_ERROR "Casil will be statically linked to the PyCasil binding.")
endif()

set(CASIL_INSTALL_STATIC ON CACHE BOOL "Install static Casil library.")
set(CASIL_INSTALL_SHARED ON CACHE BOOL "Install shared Casil library.")
set(CASIL_INSTALL_BINDING ON CACHE BOOL "Install PyCasil Python binding.")

if(CASIL_INSTALL_STATIC AND (NOT CASIL_BUILD_STATIC))
    message(FATAL_ERROR "Cannot install static library if not building it.")
endif()
if(CASIL_INSTALL_SHARED AND (NOT CASIL_BUILD_SHARED))
    message(FATAL_ERROR "Cannot install shared library if not building it.")
endif()
if(CASIL_INSTALL_BINDING AND (NOT CASIL_BUILD_BINDING))
    message(FATAL_ERROR "Cannot install binding if not building it.")
endif()

set(CASIL_USE_SYSTEM_YAMLCPP OFF CACHE BOOL "Use system's yaml-cpp library.")
set(CASIL_USE_SYSTEM_PYBIND11 OFF CACHE BOOL "Use system's pybind11 library.")

set(CASIL_LINK_STATIC_BOOST OFF CACHE BOOL "Statically link boost libraries.")

set(CASIL_PYTHON_SITEPACKS_LIBDIR "" CACHE PATH "Python site-packages directory (relative to CMAKE_INSTALL_PREFIX).")
set(CASIL_PYTHON_SITEPACKS_LIBDIR_AUTOSET OFF CACHE BOOL "Automatically determine CASIL_PYTHON_SITEPACKS_LIBDIR.")

set(CASIL_CMAKE_LICENSEDIR "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${CMAKE_PROJECT_NAME}"
    CACHE PATH "Install path for the LICENSE file (relative to CMAKE_INSTALL_PREFIX).")

set(CASIL_CMAKE_DATADIR "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
    CACHE PATH "Install path for general project data (relative to CMAKE_INSTALL_PREFIX).")

set(CASIL_BUILD_DOCUMENTATION OFF CACHE BOOL "Build library documentation with Doxygen.")
set(CASIL_BUILD_DOCUMENTATION_INTERNAL OFF CACHE BOOL "Include implementation details in the library documentation.")

#Configure source code macros

set(CASIL_DEVICE_DESCRIPTIONS_SUBDIR "device_descriptions/scpi")
set(CASIL_DEVICE_DESCRIPTIONS_BUILDDIR "${PROJECT_BINARY_DIR}/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}")
set(CASIL_DEVICE_DESCRIPTIONS_INSTALLDIR "${CASIL_CMAKE_DATADIR}/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}")

set(CASIL_DEV_DESC_DIRS ""
    CACHE PATH "Fixed path that will be compiled into the targets to search for the basil SCPI device descriptions (optional).")
set(CASIL_DEV_DESC_DIRS_AUTOSET_BUILD OFF
    CACHE BOOL "Set CASIL_DEV_DESC_DIRS to the device description folder that gets created in the build directory.")
set(CASIL_DEV_DESC_DIRS_AUTOSET_INSTALL OFF
    CACHE BOOL "Set CASIL_DEV_DESC_DIRS to the device description install path CASIL_CMAKE_DATADIR/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}.")

if(CASIL_DEV_DESC_DIRS_AUTOSET_BUILD AND CASIL_DEV_DESC_DIRS_AUTOSET_INSTALL)
    message(FATAL_ERROR "Can only auto-set CASIL_DEV_DESC_DIRS to one path at a time.")
endif()

if(CASIL_DEV_DESC_DIRS_AUTOSET_BUILD)
    set(CASIL_DEV_DESC_DIRS "${CASIL_DEVICE_DESCRIPTIONS_BUILDDIR}"
        CACHE PATH "Fixed path that will be compiled into the targets to search for the basil SCPI device descriptions (optional)." FORCE)
elseif(CASIL_DEV_DESC_DIRS_AUTOSET_INSTALL)
    set(CASIL_DEV_DESC_DIRS "${CMAKE_INSTALL_PREFIX}/${CASIL_DEVICE_DESCRIPTIONS_INSTALLDIR}"
        CACHE PATH "Fixed path that will be compiled into the targets to search for the basil SCPI device descriptions (optional)." FORCE)
endif()

set(CASIL_DEFINE_TEST_DATA_DIR OFF CACHE BOOL "Compile fixed test sources path (PROJECT_SOURCE_DIR/tests/stand-alone/casil) into unit tests.")

if(CASIL_DEFINE_TEST_DATA_DIR AND (NOT CASIL_TEST_DATA_DIR))
    set(CASIL_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/tests/stand-alone/casil")
endif()

#Configure compiler/linker flags

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CASIL_CXX_FLAGS}>")

if((NOT MSVC) AND CASIL_ENABLE_GLIBCXX_DEBUG_MODE)
    add_compile_options(-Wp,-D_GLIBCXX_DEBUG -Wp,-D_GLIBCXX_DEBUG_PEDANTIC)
endif()

add_link_options("$<$<LINK_LANGUAGE:CXX>:${CASIL_LINK_FLAGS}>")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_link_options("$<$<LINK_LANGUAGE:CXX>:${CASIL_LINK_FLAGS_DEBUG}>")
else()
    add_link_options("$<$<LINK_LANGUAGE:CXX>:${CASIL_LINK_FLAGS_RELEASE}>")
endif()

#Sources

include_directories(BEFORE "${PROJECT_SOURCE_DIR}")

include("SourceFiles.cmake")

set(PYBIND_HEADER_FILES pycasil/pycasil.h)

set(PYBIND_SOURCE_FILES pycasil/pycasil.cpp
                        pycasil/HL/hl.cpp
                        pycasil/RL/rl.cpp
                        pycasil/TL/tl.cpp
)

foreach(fileName ${HEADER_FILE_NAMES})
    list(APPEND HEADER_FILES "casil/${fileName}")
endforeach()

foreach(fileName ${SOURCE_FILE_NAMES})
    list(APPEND SOURCE_FILES "casil/${fileName}.cpp")
    list(APPEND PYBIND_SOURCE_FILES "pycasil/${fileName}_py.cpp")
endforeach()

foreach(fileName ${TESTS_FILE_NAMES})
    list(APPEND TESTS_FILES "tests/stand-alone/casil/${fileName}")
endforeach()

#External libraries

if(CASIL_LINK_STATIC_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(BUILD_TESTING OFF CACHE BOOL "")    #Disable Boost tests

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development)

if(CASIL_PYTHON_SITEPACKS_LIBDIR_AUTOSET AND (NOT CASIL_PYTHON_SITEPACKS_LIBDIR))
    if(Python_Interpreter_FOUND)
        execute_process(COMMAND "${Python_EXECUTABLE}" -c
                                "import os; import sys; import sysconfig;\
                                 sys.stdout.write(sysconfig.get_path('platlib').removeprefix(os.path.join(sys.prefix, '')));"
                        RESULT_VARIABLE PLATLIB_EXEC_RESULT OUTPUT_VARIABLE PLATLIB_EXEC_OUTPUT ERROR_QUIET)
        if(NOT PLATLIB_EXEC_RESULT)
            set(CASIL_PYTHON_SITEPACKS_LIBDIR "${PLATLIB_EXEC_OUTPUT}"
                CACHE PATH "Python site-packages directory (relative to CMAKE_INSTALL_PREFIX)." FORCE)
        else()
            message(SEND_ERROR "Could not automatically determine Python site-packages directory.")
        endif()
    else()
        message(SEND_ERROR "Could not find Python interpreter (needed to determine site-packages directory).")
    endif()
endif()

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "")
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "")
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "")
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(YAML_CPP_INSTALL OFF CACHE BOOL "")
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "")

if(CASIL_USE_SYSTEM_YAMLCPP)
    find_package(yaml-cpp 0.8.0 REQUIRED)
else()
    add_subdirectory(external/yaml-cpp)
endif()

if(CASIL_USE_SYSTEM_PYBIND11)
    find_package(pybind11 2.13.4 REQUIRED)
else()
    add_subdirectory(external/pybind11)
endif()

if(CASIL_INSTALL_BINDING AND (NOT CASIL_PYTHON_SITEPACKS_LIBDIR))
    message(FATAL_ERROR "Site-packages path to install PyCasil binding needs to be set.")
endif()

if(CASIL_BUILD_DOCUMENTATION)
    find_package(Doxygen 1.12.0 REQUIRED)
endif()

#Targets

add_library(CasilObjLib OBJECT ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(CasilObjLib PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CASIL_DEV_DESC_DIRS)
    target_compile_definitions(CasilObjLib PRIVATE "CASIL_DEV_DESC_DIRS=${CASIL_DEV_DESC_DIRS}")
endif()
target_sources(CasilObjLib PUBLIC FILE_SET HEADERS BASE_DIRS "${PROJECT_SOURCE_DIR}" FILES "${HEADER_FILES}")

if(CASIL_BUILD_SHARED)
    add_library(CasilLibShared SHARED $<TARGET_OBJECTS:CasilObjLib>)
    set_target_properties(CasilLibShared PROPERTIES OUTPUT_NAME Casil)
    set_target_properties(CasilLibShared PROPERTIES VERSION ${PROJECT_VERSION})
    target_link_libraries(CasilLibShared PRIVATE yaml-cpp)
    #target_link_libraries(CasilLibShared PRIVATE ${Boost_LIBRARIES})   #Not needed for the currently used libraries
endif()

if(CASIL_BUILD_STATIC)
    add_library(CasilLibStatic STATIC $<TARGET_OBJECTS:CasilObjLib>)
    set_target_properties(CasilLibStatic PROPERTIES OUTPUT_NAME Casil)
    set_target_properties(CasilLibStatic PROPERTIES VERSION ${PROJECT_VERSION})
    target_link_libraries(CasilLibStatic PRIVATE yaml-cpp)
    #target_link_libraries(CasilLibStatic PRIVATE ${Boost_LIBRARIES})   #Not needed for the currently used libraries
endif()

if(CASIL_BUILD_EXAMPLE)
    add_executable(CasilExample example/casil/main.cpp $<TARGET_OBJECTS:CasilObjLib>)
    target_link_libraries(CasilExample PRIVATE yaml-cpp)
endif()

if(CASIL_BUILD_BINDING)
    pybind11_add_module(CasilPython ${PYBIND_HEADER_FILES} ${PYBIND_SOURCE_FILES})
    set_target_properties(CasilPython PROPERTIES OUTPUT_NAME PyCasil)
    target_link_libraries(CasilPython PRIVATE CasilObjLib)
    target_link_libraries(CasilPython PRIVATE yaml-cpp)
    if(NOT MSVC)
        target_link_options(CasilPython PRIVATE -Wl,-z,undefs)  #Workaround: pybind11 fails to link when using 'defs' linker option
    else()
        #Don't know if need workaround here too... (probably "/FORCE:UNRESOLVED" then?)
    endif()
endif()

if(CASIL_BUILD_TESTS)
    add_executable(CasilTests tests/stand-alone/casil/tests.cpp ${TESTS_FILES} $<TARGET_OBJECTS:CasilObjLib>)
    if(NOT CASIL_LINK_STATIC_BOOST)
        target_compile_definitions(CasilTests PRIVATE BOOST_TEST_DYN_LINK)
    endif()
    if(CASIL_TEST_DATA_DIR)
        target_compile_definitions(CasilTests PRIVATE "CASIL_TEST_DATA_DIR=${CASIL_TEST_DATA_DIR}")
    endif()
    target_link_libraries(CasilTests PRIVATE yaml-cpp)
    target_link_libraries(CasilTests PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endif()

if(CASIL_BUILD_DOCUMENTATION)
    set(CASIL_DOXYFILE_IN "${PROJECT_SOURCE_DIR}/doc/casil/Doxyfile.in")
    set(CASIL_DOXYFILE "${PROJECT_BINARY_DIR}/doc/casil/Doxyfile")
    set(CASIL_DOXYGEN_PROJECT_NUMBER "${CMAKE_PROJECT_VERSION}")
    set(CASIL_DOXYGEN_TEMPLATE_DIR "${PROJECT_SOURCE_DIR}/doc/casil/template")
    set(CASIL_DOXYGEN_SOURCES_DIR "${PROJECT_SOURCE_DIR}/casil")
    if(CASIL_BUILD_DOCUMENTATION_INTERNAL)
        set(CASIL_DOXYGEN_EXTRACT_PRIVATE "YES")
        set(CASIL_DOXYGEN_INTERNAL_DOCS "YES")
        set(CASIL_DOXYGEN_ENABLED_SECTIONS "INTERNAL")
    else()
        set(CASIL_DOXYGEN_EXTRACT_PRIVATE "NO")
        set(CASIL_DOXYGEN_INTERNAL_DOCS "NO")
        set(CASIL_DOXYGEN_ENABLED_SECTIONS "")
    endif()
    configure_file("${CASIL_DOXYFILE_IN}" "${CASIL_DOXYFILE}" @ONLY)
    add_custom_target(Documentation ALL ${DOXYGEN_EXECUTABLE} "${CASIL_DOXYFILE}" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/doc/casil" VERBATIM)
endif()

#Configure additional files

#Patch and configure SCPI device descriptions
file(READ "${PROJECT_SOURCE_DIR}/misc/basil_licence_notice_patch.yaml" PATCH_NOTICE)
foreach(fileName ${SCPI_DEVICE_DESCRIPTION_FILE_NAMES})
    file(READ "${PROJECT_SOURCE_DIR}/external/basil/basil/HL/${fileName}" DEVDESCR)
    file(WRITE "${PROJECT_BINARY_DIR}/tmp_devdescr_patch/${fileName}" "${PATCH_NOTICE}")
    file(APPEND "${PROJECT_BINARY_DIR}/tmp_devdescr_patch/${fileName}" "${DEVDESCR}")
    configure_file("${PROJECT_BINARY_DIR}/tmp_devdescr_patch/${fileName}" "${CASIL_DEVICE_DESCRIPTIONS_BUILDDIR}/${fileName}" COPYONLY)
endforeach()
file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/tmp_devdescr_patch")

#Installation

install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CASIL_CMAKE_LICENSEDIR}")

install(DIRECTORY "${PROJECT_BINARY_DIR}/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}/" DESTINATION "${CASIL_DEVICE_DESCRIPTIONS_INSTALLDIR}")

install(TARGETS CasilObjLib FILE_SET HEADERS DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

if(CASIL_INSTALL_SHARED)
    install(TARGETS CasilLibShared LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(CASIL_INSTALL_STATIC)
    install(TARGETS CasilLibStatic LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(CASIL_INSTALL_BINDING)
    install(TARGETS CasilPython LIBRARY DESTINATION ${CASIL_PYTHON_SITEPACKS_LIBDIR})
endif()

if((CASIL_INSTALL_SHARED OR CASIL_INSTALL_STATIC) AND CASIL_BUILD_DOCUMENTATION)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/casil/doc/" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()

if(CASIL_BUILD_DOCUMENTATION)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()
