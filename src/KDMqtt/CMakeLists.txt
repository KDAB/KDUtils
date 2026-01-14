# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Marco Thaller <marco.thaller@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

set(SOURCES mqtt.cpp)

set(HEADERS mosquitto_wrapper.h mqtt.h)

add_library(
    KDMqtt
    ${SOURCES} ${HEADERS}
)

add_library(
    KDUtils::KDMqtt ALIAS KDMqtt
)

target_link_libraries(
    KDMqtt
    PUBLIC KDUtils::KDFoundation
)

if(TARGET unofficial::mosquitto::mosquitto)
    target_link_libraries(KDMqtt PUBLIC unofficial::mosquitto::mosquitto unofficial::mosquitto::mosquittopp)
else()
    target_link_libraries(KDMqtt PUBLIC ${MOSQUITTO_LIBRARIES})
    target_include_directories(KDMqtt PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
endif()

target_include_directories(
    KDMqtt
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
)

#
# Logging configuration
#
# Compile out some of the SPDLOG macros based on build type
target_compile_definitions(
    KDMqtt PRIVATE SPDLOG_ACTIVE_LEVEL=$<IF:$<CONFIG:Debug>,SPDLOG_LEVEL_TRACE,SPDLOG_LEVEL_WARN>
)
target_link_libraries(
    KDMqtt
    PRIVATE spdlog::spdlog
)

generate_export_header(KDMqtt BASE_NAME kdmqtt)
configure_file(${CMAKE_CURRENT_BINARY_DIR}/kdmqtt_export.h ${CMAKE_BINARY_DIR}/include/KDMqtt/kdmqtt_export.h)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kdmqtt_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDMqtt
)

foreach(file ${HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDMqtt/${dir})
endforeach()

set(project_config_in "${CMAKE_CURRENT_LIST_DIR}/cmake/KDMqttConfig.cmake.in")
set(project_config_out "${CMAKE_CURRENT_BINARY_DIR}/KDMqttConfig.cmake")

install(
    TARGETS KDMqtt
    EXPORT KDMqtt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT KDMqtt
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDMqtt
    NAMESPACE KDUtils::
    FILE KDMqttConfigTargets.cmake
)
include(CMakePackageConfigHelpers)
configure_file("${project_config_in}" "${project_config_out}" @ONLY)
install(
    FILES "${project_config_out}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDMqtt
)
install(
    FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/KDMqtt
)
