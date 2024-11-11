# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Marco Thaller <marco.thaller@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(mqtt_client_example LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    mqtt_client.cpp
)

target_link_libraries(
    ${PROJECT_NAME} KDMqtt
)

if(WIN32)
    if(Mosquitto_FOUND AND MOSQUITTO_RUNTIME_DLLS)
        foreach(MOSQUITTO_RUNTIME_DLL ${MOSQUITTO_RUNTIME_DLLS})
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MOSQUITTO_RUNTIME_DLL}"
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMENT "Copy mosquitto DLLs next to binary."
            )
        endforeach()
    endif()
endif()
