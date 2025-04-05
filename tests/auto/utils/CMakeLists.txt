# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(KDUtils-Tests)

function(add_utils_test name sources)
    add_executable(${name} ${sources})

    target_link_libraries(${name} PRIVATE KDUtils::KDUtils doctest::doctest)

    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    set_tests_properties(${name} PROPERTIES LABELS "Utils")

    if(APPLE)
        target_compile_options(${name} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

add_subdirectory(bytearray)
add_subdirectory(color)
add_subdirectory(dir)
add_subdirectory(elapsedtimer)
add_subdirectory(file)
add_subdirectory(flags)
add_subdirectory(signal)
add_subdirectory(tailwind_colors)
add_subdirectory(uri)
add_subdirectory(url)

add_feature_info(KDUtils-Test ON "Build KDUtils Tests")

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
