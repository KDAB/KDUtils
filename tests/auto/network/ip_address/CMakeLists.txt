# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-network-ip-address
    VERSION 0.1
    LANGUAGES CXX
)

add_network_test(${PROJECT_NAME} tst_ip_address.cpp)
