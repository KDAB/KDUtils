# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Shivam Kunwar <shivam.kunwar@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_executable(
    tst_signal
    tst_signal.cpp
)
target_link_libraries(
    tst_signal
    PRIVATE KDUtils KDUtils::KDFoundation doctest::doctest
)
add_test(NAME tst_signal COMMAND tst_signal)
set_tests_properties(tst_signal PROPERTIES LABELS "KDUtils")
