# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

set(SOURCES abstract_platform_window.cpp gui_application.cpp window.cpp)

set(HEADERS
    abstract_clipboard.h
    abstract_gui_platform_integration.h
    abstract_platform_window.h
    gui_application.h
    gui_events.h
    kdgui_global.h
    kdgui_keys.h
    window.h
    position.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(
        APPEND
        SOURCES
        platform/linux/xcb/linux_xcb_clipboard.cpp
        platform/linux/xcb/linux_xcb_platform_event_loop.cpp
        platform/linux/xcb/linux_xcb_platform_integration.cpp
        platform/linux/xcb/linux_xcb_platform_window.cpp
        platform/linux/xcb/linux_xkb_keyboard.cpp
        platform/linux/common/linux_xkb.cpp
        platform/linux/common/linux_xkb_keyboard_map.cpp
    )
    list(
        APPEND
        HEADERS
        platform/linux/xcb/linux_xcb_clipboard.h
        platform/linux/xcb/linux_xcb_platform_event_loop.h
        platform/linux/xcb/linux_xcb_platform_integration.h
        platform/linux/xcb/linux_xcb_platform_window.h
        platform/linux/xcb/linux_xkb_keyboard.h
        platform/linux/common/linux_xkb.h
        platform/linux/common/linux_xkb_keyboard_map.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(
        APPEND
        SOURCES
        platform/win32/win32_gui_platform_integration.cpp
        platform/win32/win32_platform_window.cpp
        platform/win32/win32_utils.cpp
        platform/win32/win32_keyboard_map.cpp
    )
    list(
        APPEND
        HEADERS
        platform/win32/win32_gui_platform_integration.h
        platform/win32/win32_keyboard_map.h
        platform/win32/win32_platform_window.h
        platform/win32/win32_utils.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(
        APPEND
        SOURCES
        platform/cocoa/cocoa_platform_integration.mm
        platform/cocoa/cocoa_platform_integration_impl.mm
        platform/cocoa/cocoa_platform_window.mm
    )
    list(
        APPEND
        HEADERS
        platform/cocoa/cocoa_platform_integration.h
        platform/cocoa/cocoa_platform_integration_impl.h
        platform/cocoa/cocoa_platform_window.h
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(
        APPEND
        SOURCES
        platform/android/android_platform_event_loop.cpp
        platform/android/android_platform_integration.cpp
        platform/android/android_platform_window.cpp
    )
    list(
        APPEND
        HEADERS
        platform/android/android_platform_event_loop.h
        platform/android/android_platform_integration.h
        platform/android/android_platform_window.h
    )
endif()

add_library(
    KDGui
    ${SOURCES}
)
add_library(
    KDUtils::KDGui ALIAS KDGui
)

set(KDGUI_PUBLIC_LIBS KDUtils::KDFoundation)

target_link_libraries(
    KDGui
    PUBLIC ${KDGUI_PUBLIC_LIBS}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    find_package(XKB REQUIRED)
    find_package(
        XCB
        COMPONENTS XKB
        REQUIRED
    )
    target_compile_definitions(KDGui PRIVATE PLATFORM_LINUX)
    target_include_directories(KDGui PRIVATE ${XCB_INCLUDE_DIRS} ${XKB_INCLUDE_DIRS})
    target_link_libraries(KDGui PRIVATE ${XCB_LIBRARIES} ${XKB_LIBRARIES})

    set(KDGUI_PLATFORM_XCB ON)

    find_package(
        Wayland
        COMPONENTS Client Cursor
        QUIET
    )
    find_package(WaylandProtocols QUIET)
    find_package(WaylandScanner QUIET)

    if(${Wayland_FOUND}
       AND ${WaylandProtocols_FOUND}
       AND ${WaylandScanner_FOUND}
    )
        target_compile_definitions(KDGui PRIVATE PLATFORM_WAYLAND)

        set(WAYLAND_HEADERS
            platform/linux/wayland/linux_wayland_clipboard.h
            platform/linux/wayland/linux_wayland_platform_integration.h
            platform/linux/wayland/linux_wayland_platform_event_loop.h
            platform/linux/wayland/linux_wayland_platform_window.h
            platform/linux/wayland/linux_wayland_platform_input.h
            platform/linux/wayland/linux_wayland_platform_output.h
        )
        set(WAYLAND_SOURCES
            platform/linux/wayland/linux_wayland_clipboard.cpp
            platform/linux/wayland/linux_wayland_platform_integration.cpp
            platform/linux/wayland/linux_wayland_platform_event_loop.cpp
            platform/linux/wayland/linux_wayland_platform_window.cpp
            platform/linux/wayland/linux_wayland_platform_input.cpp
            platform/linux/wayland/linux_wayland_platform_output.cpp
        )
        ecm_add_wayland_client_protocol(
            WAYLAND_SOURCES
            BASENAME
            xdg-shell
            PROTOCOL
            ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
        )
        ecm_add_wayland_client_protocol(
            WAYLAND_SOURCES
            BASENAME
            zwp-relative-pointer-unstable-v1
            PROTOCOL
            ${WaylandProtocols_DATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml
        )
        ecm_add_wayland_client_protocol(
            WAYLAND_SOURCES
            BASENAME
            zwp-pointer-constraints-v1
            PROTOCOL
            ${WaylandProtocols_DATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
        )
        ecm_add_wayland_client_protocol(
            WAYLAND_SOURCES
            BASENAME
            xdg-decoration-unstable-v1
            PROTOCOL
            ${WaylandProtocols_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
        )
        target_sources(KDGui PRIVATE ${WAYLAND_SOURCES} ${WAYLAND_HEADERS})
        target_include_directories(KDGui PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

        target_link_libraries(KDGui PRIVATE Wayland::Client Wayland::Cursor)

        list(APPEND HEADERS ${WAYLAND_HEADERS})
        set(KDGUI_PLATFORM_WAYLAND ON)
    endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(KDGui PRIVATE PLATFORM_WIN32)
    target_compile_definitions(KDGui PRIVATE UNICODE)
    target_compile_definitions(KDGui PUBLIC _USE_MATH_DEFINES)
    set(KDGUI_PLATFORM_WIN32 ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(KDGui PRIVATE PLATFORM_MACOS)
    set(KDGUI_PLATFORM_COCOA ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
    target_include_directories(KDGui PRIVATE ${APP_GLUE_DIR})
    set(KDGUI_PLATFORM_ANDROID ON)
endif()

target_include_directories(
    KDGui
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/smhasher> $<INSTALL_INTERFACE:include>
)

generate_export_header(KDGui BASE_NAME kdgui)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdgui_export.h ${CMAKE_BINARY_DIR}/include/KDGui/kdgui_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdgui_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h ${CMAKE_BINARY_DIR}/include/KDGui/config.h)
install(
    FILES ${CMAKE_BINARY_DIR}/include/KDGui/config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui
)

if(KDUTILS_CODE_COVERAGE)
    append_coverage_compiler_flags()
endif()

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui/${dir})
endforeach()

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDGuiConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDGuiConfig.cmake")

install(
    TARGETS KDGui
    EXPORT KDGui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDGui
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGui
    NAMESPACE KDUtils::
    FILE KDGuiConfigTargets.cmake
)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(
    FILES "${project_config_out}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDGui
)
install(
    FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDGui
)
