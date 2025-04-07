# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

cmake_minimum_required(VERSION 3.16)

project(
    KDNetwork
    DESCRIPTION "Asynchronous networking library for KDFoundation"
    LANGUAGES CXX
)

include(GenerateExportHeader)

# Define sources and headers
set(SOURCES
    socket.cpp
    socket_error.cpp
    tcp_socket.cpp
    tcp_server.cpp
    udp_socket.cpp
    dns_resolver.cpp
    ssl_socket.cpp
    ip_address.cpp
    # HTTP-related sources
    http_client.cpp
    http_cookie.cpp
    http_cookie_jar.cpp
    http_request.cpp
    http_response.cpp
    http_session.cpp
    http_parser.cpp
    network_initializer.cpp
)

set(HEADERS
    socket.h
    socket_error.h
    tcp_socket.h
    tcp_server.h
    udp_socket.h
    dns_resolver.h
    ssl_socket.h
    ip_address.h
    http_client.h
    http_cookie_jar.h
    http_cookie.h
    http_request.h
    http_response.h
    http_session.h
    http_parser.h
    network_initializer.h
)

# Add library
add_library(
    KDNetwork
    ${SOURCES} ${HEADERS}
)
add_library(
    KDUtils::KDNetwork ALIAS KDNetwork
)

# Link dependencies
target_link_libraries(
    KDNetwork
    PUBLIC KDFoundation OpenSSL::SSL OpenSSL::Crypto
    PRIVATE llhttp_static
)

# Link c-ares for asynchronous DNS resolution
if(TARGET c-ares::cares)
    target_link_libraries(KDNetwork PRIVATE c-ares::cares)
else()
    target_link_libraries(KDNetwork PRIVATE c-ares)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(KDNetwork PUBLIC ws2_32)
endif()

# Include directories
target_include_directories(
    KDNetwork
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include> $<INSTALL_INTERFACE:include>
)

# Require >=C++17 for us and downstream projects
target_compile_features(KDFoundation PUBLIC cxx_std_17)

# Generate export header
generate_export_header(KDNetwork BASE_NAME kdnetwork)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdnetwork_export.h ${CMAKE_BINARY_DIR}/include/KDNetwork/kdnetwork_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdfoundation_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDNetwork
)

if(KDUTILS_CODE_COVERAGE)
    append_coverage_compiler_flags()
endif()

# Install targets and headers
foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDNetwork/${dir})
endforeach()

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDNetworkConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDNetworkConfig.cmake")

install(
    TARGETS KDNetwork
    EXPORT KDNetwork
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDNetwork
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDNetwork
    NAMESPACE KDUtils::
    FILE KDNetworkConfigTargets.cmake
)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(
    FILES "${project_config_out}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDNetwork
)
install(
    FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDNetwork
)
