# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: AGPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

include(FetchContent)

message(STATUS "Looking for KDUtils dependencies")

# spdlog Logging Library
# spdlog needs to be installed. If already exists in the prefix, we don't want to override it, so first we try to find it
# If we don't find it, then we fetch it and install it
find_package(spdlog QUIET)
if (NOT ${spdlog_FOUND})
    message(STATUS "spdlog was not found. Fetching from git")
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.8.5
        )
    set(SPDLOG_INSTALL ON CACHE BOOL "Should install spdlog")
    FetchContent_MakeAvailable(spdlog)
endif()

# whereami library
FetchContent_Declare(
    whereami
    GIT_REPOSITORY https://github.com/gpakosz/whereami
    GIT_TAG e4b7ba1be0e9fd60728acbdd418bc7195cdd37e7 # master at 5/July/2021
    )
FetchContent_MakeAvailable(whereami)

# doctest library
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 2.4.6
    )
FetchContent_MakeAvailable(doctest)
