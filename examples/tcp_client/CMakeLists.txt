# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(tcp_client LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    client.cpp client.h main.cpp
)

target_link_libraries(
    ${PROJECT_NAME} KDUtils::KDNetwork
)
