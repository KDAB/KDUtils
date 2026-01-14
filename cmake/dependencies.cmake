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
find_package(unofficial-whereami QUIET) # vcpkg unofficial package
if(TARGET unofficial::whereami::whereami)
    add_library(whereami::whereami ALIAS unofficial::whereami::whereami)
endif()

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
    find_package(unofficial-mosquitto QUIET) # vcpkg unofficial package
    if(NOT TARGET unofficial::mosquitto::mosquitto)
        find_package(mosquitto REQUIRED) # try to use system package
    endif()
endif()
