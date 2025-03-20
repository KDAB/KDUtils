# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(filesystem_example LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    filesystem.cpp
)

target_link_libraries(
    ${PROJECT_NAME} KDGui
)
