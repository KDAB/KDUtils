# This file is part of KDGpu.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
# TODO: we will probably want to have something more verbose in the future
add_custom_command(
    OUTPUT kdutils_texts
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR} .
    COMMENT "Copy the text assets into the current source directory"
)

add_custom_target(
    KDUtils_text ALL
    DEPENDS kdutils_texts
    COMMENT "Target to copy the text assets into the current source directory"
)
