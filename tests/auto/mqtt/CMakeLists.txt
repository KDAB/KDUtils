# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Marco Thaller <marco.thaller@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(KDMqtt-Tests)

include_directories(../KDFoundation/common)

file(DOWNLOAD https://raw.githubusercontent.com/eranpeer/FakeIt/refs/tags/2.4.1/single_header/doctest/fakeit.hpp
     "${CMAKE_BINARY_DIR}/tests/auto/fakeit.h"
     EXPECTED_HASH SHA256=09be612875c0dd0c0692256739eb859e66f6e82c1f1082d50eacb670f471f63b
)

project(
    test-mqtt
    VERSION 0.1
    LANGUAGES CXX
)

add_executable(
    ${PROJECT_NAME}
    tst_mqtt.cpp
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/tests/auto/foundation/common ${CMAKE_BINARY_DIR}/tests/auto
)

target_link_libraries(
    ${PROJECT_NAME} KDMqtt doctest::doctest
)

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
set_tests_properties(${PROJECT_NAME} PROPERTIES LABELS "Mqtt")

add_feature_info(KDMqtt-Test ON "Build KDMqtt Tests")

if(KDUTILS_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME
        mqtt_coverage
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        EXECUTABLE
        ctest
        -L
        "Mqtt"
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "${CMAKE_SOURCE_DIR}/src/utils/*"
        "${CMAKE_SOURCE_DIR}/src/foundation/*"
        "${CMAKE_SOURCE_DIR}/src/3rdparty/*"
    )
    add_feature_info(Mqtt-Coverage ON "Generate Code Coverage - ninja mqtt_coverage")
endif()
