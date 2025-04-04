# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(
    test-network-tcp-socket
    VERSION 0.1
    LANGUAGES CXX
)

add_network_test(${PROJECT_NAME} tst_tcp_socket.cpp)
