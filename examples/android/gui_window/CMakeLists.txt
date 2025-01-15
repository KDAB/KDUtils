# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

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

# Declares and names the project.

project("KDGuiWindowTest")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_FIND_DEBUG_MODE TRUE)

# Creates and names a library, sets it as either STATIC or SHARED, and provides
# the relative paths to its source code. You can define multiple libraries, and
# CMake builds them for you. Gradle automatically packages shared libraries with
# your APK.

add_library(
    # Sets the name of the library.
    kdguiwindowtest
    # Sets the library as a shared library.
    SHARED
    # Provides a relative path to your source file(s).
    # SerenityNativeAndroid.cpp SerenityNativeAndroid.h
    gui_window.cpp
)

find_package(KDUtils CONFIG)

if(NOT KDUtils_FOUND)
    FetchContent_Declare(
        KDUtils
        GIT_REPOSITORY https://github.com/KDAB/KDUtils.git
        GIT_TAG main
        USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
    )

    option(KDUTILS_BUILD_TESTS "Build the tests" OFF)

    FetchContent_MakeAvailable(KDUtils)
endif()

find_package(KDFoundation CONFIG)
find_package(KDGui CONFIG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DVK_USE_PLATFORM_ANDROID_KHR")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries(
    # Specifies the target library.
    kdguiwindowtest
    vulkan
    nativewindow
    app-glue
    log
    android
    KDFoundation
    KDGui
    KDUtils
    spdlog
)
