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

project(Casil VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

#Configure options

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(NOT MSVC)
    set(CASIL_CXX_FLAGS "-Wall;-Wextra;-Wshadow;-pedantic-errors" CACHE STRING "Additional compiler flags.")
else()
    set(CASIL_CXX_FLAGS "/W4;/permissive-" CACHE STRING "Additional compiler flags.")
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
    if(NOT HEADER_FILES)
        set(HEADER_FILES "casil/${fileName}")
    else()
        set(HEADER_FILES "${HEADER_FILES}" "casil/${fileName}")
    endif()
endforeach(fileName)

foreach(fileName ${SOURCE_FILE_NAMES})
    if(NOT SOURCE_FILES)
        set(SOURCE_FILES "casil/${fileName}.cpp")
    else()
        set(SOURCE_FILES "${SOURCE_FILES}" "casil/${fileName}.cpp")
    endif()
    if(NOT PYBIND_SOURCE_FILES)
        set(PYBIND_SOURCE_FILES "pycasil/${fileName}_py.cpp")
    else()
        set(PYBIND_SOURCE_FILES "${PYBIND_SOURCE_FILES}" "pycasil/${fileName}_py.cpp")
    endif()
endforeach(fileName)

foreach(fileName ${TESTS_FILE_NAMES})
    if(NOT TESTS_FILES)
        set(TESTS_FILES "tests/stand-alone/casil/${fileName}")
    else()
        set(TESTS_FILES "${TESTS_FILES}" "tests/stand-alone/casil/${fileName}")
    endif()
endforeach(fileName)

#External libraries

if(CASIL_LINK_STATIC_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(BUILD_TESTING OFF CACHE BOOL "")

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

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${CASIL_CXX_FLAGS}>")

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

if(CASIL_BUILD_DOCUMENTATION AND DOXYGEN_FOUND)
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

if((CASIL_INSTALL_SHARED OR CASIL_INSTALL_STATIC) AND CASIL_BUILD_DOCUMENTATION AND DOXYGEN_FOUND)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/casil/doc/" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()

if(BUILD_DOCUMENTATION AND DOXYGEN_FOUND)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()
