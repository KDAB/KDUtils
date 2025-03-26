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

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/../assets
    COMMENT "Copying assets from ${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_link_libraries(
    ${PROJECT_NAME} KDGui
)
