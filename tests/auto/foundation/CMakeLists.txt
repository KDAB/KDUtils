# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: AGPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.0.0)
project(KDFoundation-Tests)

include_directories(./common)

function(add_core_test NAME SOURCES)
    add_executable(${NAME} ${SOURCES})

    target_link_libraries(${NAME} PRIVATE KDUtils::Foundation doctest)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    set_tests_properties(${NAME} PROPERTIES LABELS "Foundatation")

    if(APPLE)
        target_compile_options(${NAME} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

function(add_core_bench NAME SOURCES)
    add_executable(${NAME} ${SOURCES})
    target_link_libraries(${NAME} PRIVATE KDUtils::Foundation doctest nanobench)

    add_test(NAME ${NAME} COMMAND $<TARGET_FILE:${NAME}>)
    set_tests_properties(${NAME} PROPERTIES LABELS "Foundation")
endfunction()

add_subdirectory(constexpr_sort)
add_subdirectory(core_application)
add_subdirectory(event)
add_subdirectory(event_queue)
add_subdirectory(flags)
add_subdirectory(object)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(linux_platform_event_loop)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_subdirectory(win32_platform_event_loop)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_subdirectory(macos_platform_event_loop)
endif()

add_feature_info(KDFoundation-Tests ON "Build Tests")

if(KDUTILS_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME foundation_coverage
        BASE_DIRECTORY ${CMAKE_SOURCE_DIR}
        EXECUTABLE ctest -L "Foundation" -E test-foundation-linux-platform-event-loop
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "${CMAKE_SOURCE_DIR}/src/utils/*"
        "${CMAKE_SOURCE_DIR}/src/3rdparty/*"
    )
    add_feature_info(Foundation-Coverage ON "Generate Code Coverage - ninja foundation_coverage")
endif()
