# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.16)
project(
    KDUtils
    DESCRIPTION "A set of C++ helpers and wrappers around the C++ standard library"
    LANGUAGES CXX C
    VERSION 0.1.10
    HOMEPAGE_URL "https://github.com/KDAB/KDUtils"
)

if(ANDROID)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
    option(KDUTILS_BUILD_TESTS "Build the tests" OFF)
else()
    option(KDUTILS_BUILD_TESTS "Build the tests" ON)
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/find-modules")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0090 NEW) # Stop export(PACKAGE) from modifying the system-wide cmake package system
cmake_policy(SET CMP0117 NEW) # Do not add /GR to CMAKE_CXX_FLAGS

option(KDUTILS_BUILD_EXAMPLES "Build examples" ON)
option(KDUTILS_BUILD_NETWORK_SUPPORT "Build KDNetwork" ON)

include(FeatureSummary)
include(CMakeDependentOption)
include(cmake/dependencies.cmake)
include(GenerateExportHeader)
include(GNUInstallDirs)
include(CTest)

if(UNIX)
    cmake_dependent_option(
        KDUTILS_BUILD_MQTT_SUPPORT
        "EXPERIMENTAL: Build KDMqtt"
        ON
        "Mosquitto_FOUND"
        OFF
    )
else()
    option(KDUTILS_BUILD_MQTT_SUPPORT "EXPERIMENTAL: Build KDMqtt" OFF)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    option(KDUTILS_CODE_COVERAGE "Code Coverage" OFF)

    if(KDUTILS_CODE_COVERAGE)
        include(cmake/CodeCoverage.cmake)
    endif()
endif()

add_subdirectory(src/KDUtils)
add_subdirectory(src/KDFoundation)
add_subdirectory(src/KDGui)

if(KDUTILS_BUILD_NETWORK_SUPPORT)
    add_subdirectory(src/KDNetwork)
endif()

if(KDUTILS_BUILD_MQTT_SUPPORT)
    add_subdirectory(src/KDMqtt)
endif()

if(KDUTILS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(KDUTILS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

feature_summary(WHAT PACKAGES_FOUND ENABLED_FEATURES PACKAGES_NOT_FOUND DISABLED_FEATURES INCLUDE_QUIET_PACKAGES)
