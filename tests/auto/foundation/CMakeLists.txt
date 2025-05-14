# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(KDFoundation-Tests)

include_directories(./common)

# add a test with the specified name
function(add_core_test name sources)
    add_executable(${name} ${sources})

    target_link_libraries(${name} PRIVATE KDUtils::KDFoundation doctest::doctest)

    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    set_tests_properties(${name} PROPERTIES LABELS "Foundation")

    if(APPLE)
        target_compile_options(${name} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

# add a benchmark with the specified name
function(add_core_bench name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE KDUtils::KDFoundation doctest::doctest nanobench)

    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    set_tests_properties(${name} PROPERTIES LABELS "Foundation")
endfunction()

add_subdirectory(constexpr_sort)
add_subdirectory(core_application)
add_subdirectory(event)
add_subdirectory(event_queue)
add_subdirectory(object)
add_subdirectory(timer)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(linux_platform_event_loop)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_subdirectory(win32_platform_event_loop)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_subdirectory(macos_platform_event_loop)
endif()

add_feature_info(KDFoundation-Tests ON "Build KDFoundation Tests")

if(KDUTILS_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME
        foundation_coverage
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        EXECUTABLE
        ctest
        -L
        "Foundation"
        -E
        test-foundation-linux-platform-event-loop
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "${CMAKE_SOURCE_DIR}/src/utils/*"
        "${CMAKE_SOURCE_DIR}/src/3rdparty/*"
    )
    add_feature_info(Foundation-Coverage ON "Generate Code Coverage - ninja foundation_coverage")
endif()
