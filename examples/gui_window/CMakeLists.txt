# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(gui_window_example LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    gui_window.cpp
)

target_link_libraries(
    ${PROJECT_NAME} KDGui
)
