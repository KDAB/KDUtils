# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(KDNetwork-Tests)

include_directories(../foundation/common)

# add a test with the specified name
function(add_network_test name sources)
    add_executable(${name} ${sources})

    target_link_libraries(${name} PRIVATE KDUtils::KDNetwork doctest::doctest)

    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    set_tests_properties(${name} PROPERTIES LABELS "Network")

    if(APPLE)
        target_compile_options(${name} PRIVATE -Wno-deprecated-declarations)
    endif()
endfunction()

# add a benchmark with the specified name
function(add_network_bench name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE KDUtils::KDNetwork doctest::doctest nanobench)

    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    set_tests_properties(${name} PROPERTIES LABELS "Network")
endfunction()

add_subdirectory(tcp_socket)

add_feature_info(KDNetwork-Tests ON "Build KDNetwork Tests")

if(KDUTILS_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME
        network_coverage
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        EXECUTABLE
        ctest
        -L
        "Network"
        -E
        test-network-tcp-socket
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "${CMAKE_SOURCE_DIR}/src/utils/*"
        "${CMAKE_SOURCE_DIR}/src/3rdparty/*"
    )
    add_feature_info(Network-Coverage ON "Generate Code Coverage - ninja network_coverage")
endif()
