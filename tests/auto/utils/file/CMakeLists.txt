# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_executable(
    tst_file
    tst_file.cpp
)
target_link_libraries(
    tst_file
    PRIVATE KDUtils doctest::doctest
)
get_filename_component(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ ABSOLUTE)
target_compile_definitions(tst_file PRIVATE ASSETS="${ASSETS_DIR}/" TST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
doctest_discover_tests(
    tst_file
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
