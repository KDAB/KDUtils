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
    tst_flags
    tst_flags.cpp
)
target_link_libraries(
    tst_flags
    PRIVATE KDUtils doctest::doctest
)
doctest_discover_tests(
    tst_flags
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)
