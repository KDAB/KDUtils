# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-core-destruction-helper
    VERSION 0.1
    LANGUAGES CXX
)

add_core_test(${PROJECT_NAME} tst_destruction_helper.cpp)
