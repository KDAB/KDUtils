# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Nicolas Guichard <nicolas.guichard@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_library(WHEREAMI_LIB whereami)
find_file(WHEREAMI_HEADER whereami.h)
if(WHEREAMI_LIB AND WHEREAMI_HEADER)
    cmake_path(GET WHEREAMI_HEADER PARENT_PATH WHEREAMI_INCLUDE_DIRECTORIES)

    add_library(whereami::whereami UNKNOWN IMPORTED)
    set_target_properties(
        whereami::whereami PROPERTIES IMPORTED_LOCATION ${WHEREAMI_LIB} INTERFACE_INCLUDE_DIRECTORIES
                                                                        ${WHEREAMI_INCLUDE_DIRECTORIES}
    )
    set(Whereami_FOUND TRUE)
endif()
