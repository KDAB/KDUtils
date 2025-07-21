# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

set(SOURCES
    bytearray.cpp
    dir.cpp
    elapsedtimer.cpp
    file_mapper.cpp
    logging.cpp
    uri.cpp
    url.cpp
)

set(HEADERS
    bytearray.h
    color.h
    dir.h
    elapsedtimer.h
    file.h
    file_mapper.h
    flags.h
    logging.h
    tailwind_colors.h
    uri.h
    url.h
    kdutils_global.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    list(APPEND SOURCES platform/android/android_file.cpp)
else()
    list(APPEND SOURCES file.cpp)
endif()

add_library(
    KDUtils
    ${SOURCES} ${HEADERS}
)
add_library(
    KDUtils::KDUtils ALIAS KDUtils
)
target_link_libraries(
    KDUtils
    PUBLIC spdlog::spdlog
    PRIVATE mio::mio
)

if(ANDROID)
    target_link_libraries(KDUtils PUBLIC android)
endif()

target_include_directories(
    KDUtils
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../> $<INSTALL_INTERFACE:include/>
)

if(NOT TARGET whereami::whereami)
    target_include_directories(KDUtils PRIVATE ${whereami_SOURCE_DIR}/src)
    target_sources(KDUtils PRIVATE ${whereami_SOURCE_DIR}/src/whereami.c)
else()
    target_link_libraries(KDUtils PRIVATE whereami::whereami)
endif()

set_target_properties(
    KDUtils
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN 1
)
target_compile_features(KDUtils INTERFACE cxx_std_17) # b/c of std::filesystem (a C++17 feature) usage in public API

generate_export_header(KDUtils BASE_NAME kdutils)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdutils_export.h ${CMAKE_BINARY_DIR}/include/KDUtils/kdutils_export.h)
target_include_directories(
    KDUtils
    PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdutils_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDUtils
)

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDUtilsConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDUtilsConfig.cmake")

install(
    TARGETS KDUtils
    EXPORT KDUtils
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDUtils
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDUtils
    NAMESPACE KDUtils::
    FILE KDUtilsConfigTargets.cmake
)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(
    FILES "${project_config_out}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDUtils
)
install(
    FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDUtils
)

if(KDUTILS_CODE_COVERAGE)
    include(${PROJECT_SOURCE_DIR}/cmake/CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()
