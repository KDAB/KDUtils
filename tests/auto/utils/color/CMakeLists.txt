# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Joshua Goins <joshua.goins@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_executable(
    tst_color
    tst_color.cpp
)
target_link_libraries(
    tst_color
    PRIVATE KDUtils doctest::doctest
)
doctest_discover_tests(
    tst_color
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
