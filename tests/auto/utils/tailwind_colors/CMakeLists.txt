# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Joshua Goins <joshua.goins@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_executable(
    tst_tailwind_colors
    tst_tailwind_colors.cpp
)
target_link_libraries(
    tst_tailwind_colors
    PRIVATE KDUtils doctest::doctest
)
doctest_discover_tests(
    tst_tailwind_colors
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
