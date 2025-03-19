# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.18.1)

file(TO_CMAKE_PATH "${ANDROID_NDK}" ANDROID_NDK_PATH)

set(APP_GLUE_DIR ${ANDROID_NDK_PATH}/sources/android/native_app_glue)
include_directories(${APP_GLUE_DIR})
add_library(
    app-glue STATIC
    ${APP_GLUE_DIR}/android_native_app_glue.c
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(NOT KDUtils_FOUND)
    message(STATUS "KDUtils was not found. Fetching from git")
    set(FETCHCONTENT_SOURCE_DIR_KDUTILS ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    FetchContent_Declare(
        KDUtils
        GIT_REPOSITORY https://github.com/KDAB/KDUtils.git
        GIT_TAG main
    )

    option(KDUTILS_BUILD_TESTS "Build the tests" OFF)

    FetchContent_MakeAvailable(KDUtils)
endif()

project("filesystem")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_FIND_DEBUG_MODE TRUE)

add_library(
    # Sets the name of the library.
    filesystem
    # Sets the library as a shared library.
    SHARED
    # Provides a relative path to your source file(s).
    ../filesystem.cpp
)

target_compile_definitions(filesystem PRIVATE PLATFORM_ANDROID)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries(
    filesystem
    nativewindow
    app-glue
    log
    android
    KDFoundation
    KDGui
    KDUtils
    spdlog
)
