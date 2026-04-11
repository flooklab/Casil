#[[
##################################################################################################
##
##  Copyright (C) 2024–2026 M. Frohne
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

cmake_minimum_required(VERSION 3.23)

#Define a macro to predictably initialize cache variables
macro(init_cache_variable VAR_NAME VAR_TYPE VAR_VALUE VAR_HELP)
    if(NOT DEFINED CACHE{${VAR_NAME}})
        if(DEFINED ${VAR_NAME})
            set(${VAR_NAME} ${${VAR_NAME}} CACHE ${VAR_TYPE} ${VAR_HELP})
        else()
            set(${VAR_NAME} ${VAR_VALUE} CACHE ${VAR_TYPE} ${VAR_HELP})
        endif()
    endif()
    unset(${VAR_NAME})
endmacro()

#Set/tweak CMake defaults for build type and compiler flags

init_cache_variable(CMAKE_BUILD_TYPE STRING "Release" "Choose the type of build.")

if(NOT MSVC)
    set(DEFAULT_FLAGS "")
    set(DEFAULT_FLAGS_DEBUG "-g -Og")
    set(DEFAULT_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    set(DEFAULT_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(DEFAULT_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
else()
    #Those MSVC flags are probably not tested
    set(DEFAULT_FLAGS "")
    set(DEFAULT_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
    set(DEFAULT_FLAGS_MINSIZEREL "/MD /O1 /Ob1 /DNDEBUG")
    set(DEFAULT_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG")
    set(DEFAULT_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Ob1 /DNDEBUG")
endif()

init_cache_variable(CMAKE_CXX_FLAGS STRING "${DEFAULT_FLAGS}"
                    "Flags used by the CXX compiler during all build types.")
init_cache_variable(CMAKE_CXX_FLAGS_DEBUG STRING "${DEFAULT_FLAGS_DEBUG}"
                    "Flags used by the CXX compiler during DEBUG builds.")
init_cache_variable(CMAKE_CXX_FLAGS_MINSIZEREL STRING "${DEFAULT_FLAGS_MINSIZEREL}"
                    "Flags used by the CXX compiler during MINSIZEREL builds.")
init_cache_variable(CMAKE_CXX_FLAGS_RELEASE STRING "${DEFAULT_FLAGS_RELEASE}"
                    "Flags used by the CXX compiler during RELEASE builds.")
init_cache_variable(CMAKE_CXX_FLAGS_RELWITHDEBINFO STRING "${DEFAULT_FLAGS_RELWITHDEBINFO}"
                    "Flags used by the CXX compiler during RELWITHDEBINFO builds.")

project(Casil VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

#Configure options

if(NOT MSVC)
    string(CONCAT TMP_CXX_FLAGS "-Wall;-Wextra;-Wshadow;-pedantic-errors;"
                                "-pipe;-march=x86-64-v2;-mtune=generic;-Wp,-D_FORTIFY_SOURCE=3;-Wp,-D_GLIBCXX_ASSERTIONS;"
                                "-fno-plt;-fstack-protector-strong;-fstack-clash-protection;-fcf-protection=full;-flto=auto")
    init_cache_variable(CASIL_CXX_FLAGS STRING "${TMP_CXX_FLAGS}" "Additional compiler flags.")
    init_cache_variable(CASIL_LINK_FLAGS STRING
                        "-Wl,--sort-common;-Wl,--as-needed;-Wl,-z,defs;-Wl,-z,relro;-Wl,-z,now;-Wl,-z,pack-relative-relocs"
                        "Additional linker flags.")
    init_cache_variable(CASIL_LINK_FLAGS_DEBUG STRING "" "Additional linker flags to add for DEBUG builds.")
    init_cache_variable(CASIL_LINK_FLAGS_RELEASE STRING "-Wl,-O1" "Additional linker flags to add for non-DEBUG builds.")
else()
    #Those MSVC flags are probably not tested
    string(CONCAT TMP_CXX_FLAGS "/W4;/permissive-;"
                                "/arch:SSE4.2;"
                                "/GL;/GS;/guard:cf;/guard:ehcont")
    init_cache_variable(CASIL_CXX_FLAGS STRING "${TMP_CXX_FLAGS}" "Additional compiler flags.")
    init_cache_variable(CASIL_LINK_FLAGS STRING "/CETCOMPAT;/DYNAMICBASE;/OPT;/LTCG" "Additional linker flags.")
    init_cache_variable(CASIL_LINK_FLAGS_DEBUG STRING "/DEBUG" "Additional linker flags to add for DEBUG builds.")
    init_cache_variable(CASIL_LINK_FLAGS_RELEASE STRING "" "Additional linker flags to add for non-DEBUG builds.")
endif()

if(NOT MSVC)
    init_cache_variable(CASIL_ENABLE_GLIBCXX_DEBUG_MODE BOOL OFF "Add compiler flags to enable the debug mode of libstdc++.")
endif()

init_cache_variable(CASIL_BUILD_STATIC BOOL ON "Build static Casil library.")
init_cache_variable(CASIL_BUILD_SHARED BOOL ON "Build shared Casil library.")
init_cache_variable(CASIL_BUILD_BINDING BOOL ON "Build PyCasil Python binding.")
init_cache_variable(CASIL_BUILD_EXAMPLE BOOL ON "Build example executable.")
init_cache_variable(CASIL_BUILD_TESTS BOOL ON "Build Casil unit tests.")

if((NOT CASIL_BUILD_STATIC) AND (NOT CASIL_BUILD_SHARED))
    message(FATAL_ERROR "Must build at least one version of the library (shared/static).")
endif()

if(CASIL_BUILD_BINDING AND (NOT CASIL_BUILD_STATIC))
    message(FATAL_ERROR "Casil will be statically linked to the PyCasil binding.")
endif()

init_cache_variable(CASIL_INSTALL_STATIC BOOL ON "Install static Casil library.")
init_cache_variable(CASIL_INSTALL_SHARED BOOL ON "Install shared Casil library.")
init_cache_variable(CASIL_INSTALL_BINDING BOOL ON "Install PyCasil Python binding.")

if(CASIL_INSTALL_STATIC AND (NOT CASIL_BUILD_STATIC))
    message(FATAL_ERROR "Cannot install static library if not building it.")
endif()
if(CASIL_INSTALL_SHARED AND (NOT CASIL_BUILD_SHARED))
    message(FATAL_ERROR "Cannot install shared library if not building it.")
endif()
if(CASIL_INSTALL_BINDING AND (NOT CASIL_BUILD_BINDING))
    message(FATAL_ERROR "Cannot install binding if not building it.")
endif()

init_cache_variable(CASIL_USE_SYSTEM_YAMLCPP BOOL OFF "Use system's yaml-cpp library.")
init_cache_variable(CASIL_USE_SYSTEM_PYBIND11 BOOL OFF "Use system's pybind11 library.")

init_cache_variable(CASIL_LINK_STATIC_BOOST BOOL OFF "Statically link boost libraries.")

init_cache_variable(CASIL_PYTHON_SITEPACKS_LIBDIR PATH "" "Python site-packages directory (relative to CMAKE_INSTALL_PREFIX).")
init_cache_variable(CASIL_PYTHON_SITEPACKS_LIBDIR_AUTOSET BOOL OFF "Automatically determine CASIL_PYTHON_SITEPACKS_LIBDIR.")

init_cache_variable(CASIL_CMAKE_LICENSEDIR PATH "${CMAKE_INSTALL_DATAROOTDIR}/licenses/${CMAKE_PROJECT_NAME}"
                    "Install path for the LICENSE file (relative to CMAKE_INSTALL_PREFIX).")

init_cache_variable(CASIL_CMAKE_DATADIR PATH "${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}"
                    "Install path for general project data (relative to CMAKE_INSTALL_PREFIX).")

init_cache_variable(CASIL_BUILD_DOCUMENTATION BOOL OFF "Build library documentation with Doxygen.")
init_cache_variable(CASIL_BUILD_DOCUMENTATION_INTERNAL BOOL OFF "Include implementation details in the library documentation.")

#Configure source code macros

set(CASIL_DEVICE_DESCRIPTIONS_SUBDIR "device_descriptions/scpi")
set(CASIL_DEVICE_DESCRIPTIONS_BUILDDIR "${PROJECT_BINARY_DIR}/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}")
set(CASIL_DEVICE_DESCRIPTIONS_INSTALLDIR "${CASIL_CMAKE_DATADIR}/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}")

init_cache_variable(CASIL_DEV_DESC_DIRS PATH ""
                    "Fixed path that will be compiled into the targets to search for the basil SCPI device descriptions (optional).")
init_cache_variable(CASIL_DEV_DESC_DIRS_AUTOSET_BUILD BOOL OFF
                    "Set CASIL_DEV_DESC_DIRS to the device description folder that gets created in the build directory.")
init_cache_variable(CASIL_DEV_DESC_DIRS_AUTOSET_INSTALL BOOL OFF
                    "Set CASIL_DEV_DESC_DIRS to the device description install path CASIL_CMAKE_DATADIR/${CASIL_DEVICE_DESCRIPTIONS_SUBDIR}.")

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

init_cache_variable(CASIL_DEFINE_TEST_DATA_DIR BOOL OFF
                    "Compile fixed test sources path (PROJECT_SOURCE_DIR/tests/stand-alone/casil) into unit tests.")

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

if(NOT MSVC)
    add_link_options("$<$<LINK_LANGUAGE:CXX>:${CASIL_CXX_FLAGS}>")
endif()

#Sources

include_directories(BEFORE "${PROJECT_SOURCE_DIR}")

include("SourceFiles.cmake")

set(PYBIND_HEADER_FILES pycasil/pycasil.h
                        pycasil/pycasil_utils.h)

set(PYBIND_SOURCE_FILES pycasil/pycasil.cpp
                        pycasil/pycasil_utils.cpp
                        pycasil/HL/hl.cpp
                        pycasil/RL/rl.cpp
                        pycasil/TL/tl.cpp
)

foreach(fileName ${HEADER_FILE_NAMES})
    list(APPEND HEADER_FILES "casil/${fileName}")
    if(NOT ("${fileName}" IN_LIST HEADER_FILE_NAMES_EXCLUDE_INSTALL))
        list(APPEND HEADER_FILES_INSTALL "casil/${fileName}")
    endif()
endforeach()

foreach(fileName ${SOURCE_FILE_NAMES})
    list(APPEND SOURCE_FILES "casil/${fileName}.cpp")
    list(APPEND PYBIND_SOURCE_FILES "pycasil/${fileName}_py.cpp")
endforeach()

foreach(fileName ${TESTS_FILE_NAMES})
    list(APPEND TESTS_FILES "tests/stand-alone/casil/${fileName}")
endforeach()

#External libraries

if(NOT MSVC)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)

if(CASIL_LINK_STATIC_BOOST)
    set(Boost_USE_STATIC_LIBS ON)
endif()

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)  #Disable Boost tests

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)           #Use BoostConfig.cmake
endif()

if(CASIL_BUILD_TESTS)
    find_package(Boost 1.70 REQUIRED COMPONENTS unit_test_framework)
else()
    find_package(Boost 1.70 REQUIRED)
endif()

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

if(CASIL_USE_SYSTEM_YAMLCPP)
    find_package(yaml-cpp 0.8.0 REQUIRED)
else()
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)
    set(YAML_ENABLE_PIC ON CACHE BOOL "" FORCE)

    add_subdirectory(external/yaml-cpp)
endif()

if(CASIL_USE_SYSTEM_PYBIND11)
    find_package(pybind11 3.0.1 REQUIRED)
else()
    set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)
    set(PYBIND11_TEST OFF CACHE BOOL "" FORCE)
    set(PYBIND11_NOPYTHON OFF CACHE BOOL "" FORCE)
    set(PYBIND11_SIMPLE_GIL_MANAGEMENT OFF CACHE BOOL "" FORCE)
    set(PYBIND11_USE_CROSSCOMPILING OFF CACHE BOOL "" FORCE)

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
target_sources(CasilObjLib PUBLIC FILE_SET HEADERS BASE_DIRS "${PROJECT_SOURCE_DIR}" FILES "${HEADER_FILES_INSTALL}")

if(CASIL_BUILD_SHARED)
    add_library(CasilLibShared SHARED $<TARGET_OBJECTS:CasilObjLib>)
    set_target_properties(CasilLibShared PROPERTIES OUTPUT_NAME Casil)
    set_target_properties(CasilLibShared PROPERTIES VERSION ${PROJECT_VERSION})
    target_link_libraries(CasilLibShared PRIVATE Threads::Threads)
    target_link_libraries(CasilLibShared PRIVATE yaml-cpp)
    #target_link_libraries(CasilLibShared PRIVATE ${Boost_LIBRARIES})   #Not needed for the currently used libraries
endif()

if(CASIL_BUILD_STATIC)
    add_library(CasilLibStatic STATIC $<TARGET_OBJECTS:CasilObjLib>)
    set_target_properties(CasilLibStatic PROPERTIES OUTPUT_NAME Casil)
    set_target_properties(CasilLibStatic PROPERTIES VERSION ${PROJECT_VERSION})
    target_link_libraries(CasilLibStatic PRIVATE Threads::Threads)
    target_link_libraries(CasilLibStatic PRIVATE yaml-cpp)
    #target_link_libraries(CasilLibStatic PRIVATE ${Boost_LIBRARIES})   #Not needed for the currently used libraries
endif()

if(CASIL_BUILD_EXAMPLE)
    add_executable(CasilExample example/casil/main.cpp $<TARGET_OBJECTS:CasilObjLib>)
    target_link_libraries(CasilExample PRIVATE Threads::Threads)
    target_link_libraries(CasilExample PRIVATE yaml-cpp)
endif()

if(CASIL_BUILD_BINDING)
    pybind11_add_module(CasilPython ${PYBIND_HEADER_FILES} ${PYBIND_SOURCE_FILES})
    set_target_properties(CasilPython PROPERTIES OUTPUT_NAME PyCasil)
    target_link_libraries(CasilPython PRIVATE CasilObjLib)
    target_link_libraries(CasilPython PRIVATE Threads::Threads)
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
    target_link_libraries(CasilTests PRIVATE Threads::Threads)
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
    add_custom_target(Documentation ALL Doxygen::doxygen "${CASIL_DOXYFILE}" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/doc/casil" VERBATIM)
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
    if(NOT MSVC)
        install(TARGETS CasilLibShared LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    else()
        install(TARGETS CasilLibShared RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        install(TARGETS CasilLibShared ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    endif()
endif()

if(CASIL_INSTALL_STATIC)
    install(TARGETS CasilLibStatic ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
endif()

if(CASIL_INSTALL_BINDING)
    install(TARGETS CasilPython LIBRARY DESTINATION ${CASIL_PYTHON_SITEPACKS_LIBDIR})
endif()

if((CASIL_INSTALL_SHARED OR CASIL_INSTALL_STATIC) AND CASIL_BUILD_DOCUMENTATION)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/casil/doc/" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()
