# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

include(FetchContent)

message(STATUS "Looking for KDUtils dependencies")

find_package(spdlog REQUIRED)
find_package(KDBindings REQUIRED)

# Following two packages are still acquired via fetchcontent because they aren't
# available in brew on macOS so the non-vcpkg build would be hard to set up there

# whereami library
find_package(whereami QUIET)
if(NOT TARGET whereami::whereami)
    fetchcontent_declare(
        whereami
        GIT_REPOSITORY https://github.com/gpakosz/whereami
        GIT_TAG e4b7ba1be0e9fd60728acbdd418bc7195cdd37e7 # master at 5/July/2021
    )
    fetchcontent_makeavailable(whereami)
endif()

# mio header-only lib (provides memory mapping for files)
find_package(mio QUIET)
if(NOT TARGET mio::mio)
    fetchcontent_declare(
        mio
        GIT_REPOSITORY https://github.com/mandreyel/mio.git
        GIT_TAG 8b6b7d878c89e81614d05edca7936de41ccdd2da # March 3rd 2023
    )
    fetchcontent_makeavailable(mio)
endif()

if(KDUTILS_BUILD_MQTT_SUPPORT)
    find_package(mosquitto REQUIRED)
endif()

# OpenSSL library
if(KDUTILS_BUILD_NETWORK_SUPPORT)
    find_package(OpenSSL QUIET)
    if(NOT TARGET OpenSSL::SSL)
        message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL.")
    endif()

    # c-ares library for asynchronous DNS resolution
    find_package(c-ares QUIET)
    if(NOT TARGET c-ares::cares)
        FetchContent_Declare(
            c-ares
            GIT_REPOSITORY https://github.com/c-ares/c-ares.git
            GIT_TAG b82840329a4081a1f1b125e6e6b760d4e1237b52 # v1.34.4
        )
        set(CARES_STATIC ON)
        set(CARES_SHARED OFF)
        set(CARES_BUILD_TOOLS OFF)
        set(CARES_INSTALL ON)
        FetchContent_MakeAvailable(c-ares)
    endif()

    # llhttp library
    find_package(llhttp QUIET)
    if(NOT TARGET llhttp)
        FetchContent_Declare(llhttp URL "https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.2.1.tar.gz")

        set(BUILD_SHARED_LIBS
            OFF
            CACHE INTERNAL ""
        )
        set(BUILD_STATIC_LIBS
            ON
            CACHE INTERNAL ""
        )

        FetchContent_MakeAvailable(llhttp)
    endif()

    # nlohmann json library
    find_package(nlohmann_json QUIET)
    if(NOT TARGET nlohmann_json)
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG 9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03 # v3.11.3
        )
        FetchContent_MakeAvailable(json)
    endif()
endif()
