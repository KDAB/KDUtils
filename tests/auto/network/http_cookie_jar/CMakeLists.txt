# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.

project(
    test-network-http-cookie-jar
    VERSION 0.1
    LANGUAGES CXX
)

add_network_test(${PROJECT_NAME} tst_http_cookie_jar.cpp)
