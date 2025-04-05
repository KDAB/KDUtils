# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-utils-dir
    VERSION 0.1
    LANGUAGES CXX
)
add_utils_test(${PROJECT_NAME} tst_dir.cpp)

get_filename_component(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ ABSOLUTE)
target_compile_definitions(
    test-utils-dir PRIVATE TST_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                           EXECUTABLE_DIR="$<TARGET_FILE_DIR:test-utils-dir>"
)
