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
    test-utils-file
    VERSION 0.1
    LANGUAGES CXX
)
add_utils_test(${PROJECT_NAME} tst_file.cpp)
get_filename_component(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ ABSOLUTE)
target_compile_definitions(test-utils-file PRIVATE ASSETS="${ASSETS_DIR}/" TST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
