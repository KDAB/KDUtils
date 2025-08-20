# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-gui-gui-application
    VERSION 0.1
    LANGUAGES CXX
)

add_executable(
    ${PROJECT_NAME}
    tst_gui_application.cpp
)

target_link_libraries(
    ${PROJECT_NAME} KDGui doctest::doctest
)

add_test(NAME ${PROJECT_NAME} COMMAND $<TARGET_FILE:${PROJECT_NAME}>)
set_tests_properties(${PROJECT_NAME} PROPERTIES LABELS "Gui")
