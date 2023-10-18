# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_feature_info(KDUtils-Test ON "Build KDUtils Tests")

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

add_subdirectory(bytearray)
add_subdirectory(color)
add_subdirectory(file)
add_subdirectory(dir)
add_subdirectory(elapsedtimer)
add_subdirectory(flags)
add_subdirectory(url)
add_subdirectory(logger)
add_subdirectory(tailwind_colors)

if(KDUTILS_CODE_COVERAGE)
    include(${PROJECT_SOURCE_DIR}/cmake/CodeCoverage.cmake)
    setup_target_for_coverage_gcovr_html(
        NAME
        KDUtils_coverage
        EXECUTABLE
        ctest
        -L
        "KDUtils"
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
    )
endif()
