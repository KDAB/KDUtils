# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# doctest library
find_package(doctest CONFIG REQUIRED)
# Apply https://github.com/doctest/doctest/pull/812
# to be able to #include <doctest.h> instead of #include <doctest/doctest.h>
get_target_property(DOCTEST_INTERFACE_INCLUDE_DIRECTORIES doctest::doctest INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(
    doctest::doctest SYSTEM
    INTERFACE "${DOCTEST_INTERFACE_INCLUDE_DIRECTORIES};${DOCTEST_INTERFACE_INCLUDE_DIRECTORIES}/doctest"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_compile_definitions(PLATFORM_ANDROID)
    set(PLATFORM_NAME "PLATFORM_ANDROID")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(PLATFORM_LINUX)
    set(PLATFORM_NAME "PLATFORM_LINUX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(PLATFORM_WIN32)
    set(PLATFORM_NAME "PLATFORM_WIN32")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_compile_definitions(PLATFORM_MACOS)
    set(PLATFORM_NAME "PLATFORM_MACOS")
endif()

add_subdirectory(auto/utils)
add_subdirectory(auto/foundation)
add_subdirectory(auto/gui)
if(KDUTILS_BUILD_MQTT_SUPPORT)
    add_subdirectory(auto/mqtt)
endif()
