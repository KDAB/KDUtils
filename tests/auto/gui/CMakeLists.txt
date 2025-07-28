# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(KDGui-Tests)

include_directories(../KDFoundation/common)

add_subdirectory(gui_application)
add_subdirectory(position)
add_subdirectory(window)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_subdirectory(linux_xcb_platform_window)
endif()

add_feature_info(KDGui-Tests ON "Build KDGui Tests")

if(KDUTILS_CODE_COVERAGE)
    setup_target_for_coverage_gcovr_html(
        NAME
        gui_coverage
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
        EXECUTABLE
        ctest
        -L
        "Gui"
        EXCLUDE
        "${CMAKE_BINARY_DIR}/_deps/*"
        "${CMAKE_BINARY_DIR}/src/*"
        "${CMAKE_SOURCE_DIR}/src/utils/*"
        "${CMAKE_SOURCE_DIR}/src/foundation/*"
        "${CMAKE_SOURCE_DIR}/src/3rdparty/*"
    )
    add_feature_info(Gui-Coverage ON "Generate Code Coverage - ninja gui_coverage")
endif()
