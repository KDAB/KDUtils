# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: AGPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

set(SOURCES
    abstract_platform_window.cpp
    gui_application.cpp
    window.cpp
)

set(HEADERS
    abstract_gui_platform_integration.h
    abstract_platform_window.h
    gui_application.h
    gui_events.h
    kdgui_global.h
    kdgui_keys.h
    window_extent_watcher.h
    window.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        platform/linux/xcb/linux_xcb_platform_event_loop.cpp
        platform/linux/xcb/linux_xcb_platform_integration.cpp
        platform/linux/xcb/linux_xcb_platform_window.cpp
        platform/linux/xcb/linux_xkb_keyboard.cpp
        platform/linux/common/linux_xkb.cpp
        platform/linux/common/linux_xkb_keyboard_map.cpp
    )
    list(APPEND HEADERS
        platform/linux/xcb/linux_xcb_platform_event_loop.h
        platform/linux/xcb/linux_xcb_platform_integration.h
        platform/linux/xcb/linux_xcb_platform_window.h
        platform/linux/xcb/linux_xkb_keyboard.h
        platform/linux/common/linux_xkb.h
        platform/linux/common/linux_xkb_keyboard_map.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SOURCES
        platform/win32/win32_gui_platform_integration.cpp
        platform/win32/win32_platform_window.cpp
        platform/win32/win32_utils.cpp
        platform/win32/win32_keyboard_map.cpp
    )
    list(APPEND HEADERS
        platform/win32/win32_gui_platform_integration.h
        platform/win32/win32_keyboard_map.h
        platform/win32/win32_platform_window.h
        platform/win32/win32_utils.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND SOURCES
        platform/cocoa/cocoa_platform_integration.mm
        platform/cocoa/cocoa_platform_integration_impl.mm
        platform/cocoa/cocoa_platform_window.mm
    )
    list(APPEND HEADERS
        platform/cocoa/cocoa_platform_integration.h
        platform/cocoa/cocoa_platform_integration_impl.h
        platform/cocoa/cocoa_platform_window.h
    )
endif()

add_library(KDGui SHARED
    ${SOURCES}
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
)
add_library(KDUtils::Gui ALIAS KDGui)

set(KDGUI_PUBLIC_LIBS
    KDUtils::Foundation
)

target_link_libraries(KDGui
    PUBLIC
    ${KDGUI_PUBLIC_LIBS}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(XKB REQUIRED)
    find_package(XCB COMPONENTS XKB REQUIRED)
    target_compile_definitions(KDGui PRIVATE PLATFORM_LINUX)
    target_include_directories(KDGui PRIVATE ${XKB_INCLUDE_DIRS})
    target_link_libraries(KDGui PRIVATE XCB::XCB XCB::XKB ${XKB_LIBRARIES})

    find_package(Wayland COMPONENTS Client Cursor QUIET)
    find_package(WaylandProtocols QUIET)
    find_package(WaylandScanner QUIET)

    if(${Wayland_FOUND} AND ${WaylandProtocols_FOUND} AND ${WaylandScanner_FOUND})
        target_compile_definitions(KDGui PRIVATE PLATFORM_WAYLAND)

        set(WAYLAND_HEADERS platform/linux/wayland/linux_wayland_platform_integration.h
            platform/linux/wayland/linux_wayland_platform_event_loop.h
            platform/linux/wayland/linux_wayland_platform_window.h
            platform/linux/wayland/linux_wayland_platform_input.h
            platform/linux/wayland/linux_wayland_platform_output.h)
        set(WAYLAND_SOURCES platform/linux/wayland/linux_wayland_platform_integration.cpp
            platform/linux/wayland/linux_wayland_platform_event_loop.cpp
            platform/linux/wayland/linux_wayland_platform_window.cpp
            platform/linux/wayland/linux_wayland_platform_input.cpp
            platform/linux/wayland/linux_wayland_platform_output.cpp)
        ecm_add_wayland_client_protocol(WAYLAND_SOURCES
            BASENAME xdg-shell
            PROTOCOL ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml)
        ecm_add_wayland_client_protocol(WAYLAND_SOURCES
            BASENAME zwp-relative-pointer-unstable-v1
            PROTOCOL ${WaylandProtocols_DATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml)
        ecm_add_wayland_client_protocol(WAYLAND_SOURCES
            BASENAME zwp-pointer-constraints-v1
            PROTOCOL ${WaylandProtocols_DATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml)
        target_sources(KDGui PRIVATE ${WAYLAND_SOURCES} ${WAYLAND_HEADERS})
        target_include_directories(KDGui PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

        target_link_libraries(KDGui PRIVATE Wayland::Client Wayland::Cursor)

        list(APPEND HEADERS ${WAYLAND_HEADERS})
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(KDGui PRIVATE PLATFORM_WIN32)
    target_compile_definitions(KDGui PRIVATE UNICODE)
    target_compile_definitions(KDGui PUBLIC _USE_MATH_DEFINES)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(KDGui PRIVATE PLATFORM_MACOS)
endif()

target_include_directories(KDGui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/smhasher>
    $<INSTALL_INTERFACE:include>
)

generate_export_header(KDGui BASE_NAME kdgui)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdgui_export.h ${CMAKE_BINARY_DIR}/include/KDGui/kdgui_export.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgui_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui)

if(KDUTILS_CODE_COVERAGE)
    append_coverage_compiler_flags()
endif()

add_feature_info(KDGui ON "Build Library")

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui/${dir})
endforeach()
