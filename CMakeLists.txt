# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: AGPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.12)
project(KDUtils
    DESCRIPTION "A set of C++ helpers and wrappers around the C++ standard library"
    LANGUAGES CXX C
    VERSION 0.95
    HOMEPAGE_URL "https://github.com/KDAB/KDUtils"
)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

include(FeatureSummary)
include(CMakeDependentOption)
include(cmake/dependencies.cmake)
include(GenerateExportHeader)
include(GNUInstallDirs)
include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/KDUtils)

option(KDUTILS_BUILD_TESTS ON)
if (KDUTILS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
