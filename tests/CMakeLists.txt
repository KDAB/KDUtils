# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# doctest library
find_package(doctest QUIET)
if(TARGET doctest::doctest)
    # Apply https://github.com/doctest/doctest/pull/812
    # to be able to #include <doctest.h> instead of #include <doctest/doctest.h>
    get_target_property(DOCTEST_INTERFACE_INCLUDE_DIRECTORIES doctest::doctest INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(
        doctest::doctest SYSTEM
        INTERFACE "${DOCTEST_INTERFACE_INCLUDE_DIRECTORIES};${DOCTEST_INTERFACE_INCLUDE_DIRECTORIES}/doctest"
    )
else()
    fetchcontent_declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG 3a01ec37828affe4c9650004edb5b304fb9d5b75 # dev branch commit with CMake compatibility fix
    )
    fetchcontent_makeavailable(doctest)

    list(APPEND CMAKE_MODULE_PATH ${doctest_SOURCE_DIR}/scripts/cmake)

    target_include_directories(
        doctest SYSTEM INTERFACE $<BUILD_INTERFACE:${doctest_SOURCE_DIR}/doctest>
                                 $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/KDUtils/doctest>
    )

    # Copy all include directories to SYSTEM property so we don't get warnings from doctest
    #TODO(cmake >=3.25): remove these two lines and add SYSTEM to doctest's FetchContent_Declare
    get_target_property(DOCTEST_IID doctest INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(doctest PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${DOCTEST_IID}")

    if(APPLE)
        target_compile_options(doctest INTERFACE -Wno-deprecated-declarations)
    endif()

    install(DIRECTORY ${doctest_SOURCE_DIR}/doctest/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDUtils/doctest)
endif()

add_subdirectory(auto/utils)
add_subdirectory(auto/foundation)
add_subdirectory(auto/gui)
if(KDUTILS_BUILD_NETWORK_SUPPORT)
    add_subdirectory(auto/network)
endif()
if(KDUTILS_BUILD_MQTT_SUPPORT)
    add_subdirectory(auto/mqtt)
endif()
