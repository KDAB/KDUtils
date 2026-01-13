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
add_test(NAME tst_file COMMAND tst_file)
set_tests_properties(tst_file PROPERTIES LABELS "KDUtils")
