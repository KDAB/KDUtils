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
    tst_url
    tst_url.cpp
)
target_link_libraries(
    tst_url
    PRIVATE KDUtils doctest::doctest
)
get_filename_component(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../assets/ ABSOLUTE)
target_compile_definitions(tst_url PRIVATE ASSETS="${ASSETS_DIR}/")
doctest_discover_tests(
    tst_url
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
