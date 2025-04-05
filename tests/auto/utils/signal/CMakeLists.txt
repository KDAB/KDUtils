# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Shivam Kunwar <shivam.kunwar@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-utils-signal
    VERSION 0.1
    LANGUAGES CXX
)
add_utils_test(${PROJECT_NAME} tst_signal.cpp)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE KDUtils::KDFoundation
)
