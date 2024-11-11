# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Marco Thaller <marco.thaller@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

find_file(
    MOSQUITTO_HEADER
    NAMES mosquitto.h
    PATHS /usr/include
          /usr/local/include
          /usr/local/opt/mosquitto/include
          $ENV{PROGRAMFILES}/mosquitto/devel
          $ENV{PROGRAMFILES\(X86\)}/mosquitto/devel
)

if(UNIX)
    find_library(
        MOSQUITTO_LIBRARY
        NAMES mosquitto
        PATHS /usr/lib /usr/local/lib /usr/local/opt/mosquitto/lib
    )

    if(MOSQUITTO_HEADER AND MOSQUITTO_LIBRARY)
        set(Mosquitto_FOUND TRUE)
    endif()
endif()

if(WIN32)
    find_file(
        MOSQUITTO_DLL
        NAMES mosquitto.dll
        PATHS $ENV{PROGRAMFILES}/mosquitto $ENV{PROGRAMFILES\(X86\)}/mosquitto
    )

    find_library(
        MOSQUITTO_LIBRARY
        NAMES mosquitto
        PATHS $ENV{PROGRAMFILES}/mosquitto/devel $ENV{PROGRAMFILES\(X86\)}/mosquitto/devel
    )

    file(GLOB MOSQUITTO_RUNTIME_DLLS "$ENV{PROGRAMFILES}/mosquitto/*.dll" "$ENV{PROGRAMFILES\(X86\)}/mosquitto/*.dll")

    if(MOSQUITTO_HEADER
       AND MOSQUITTO_DLL
       AND MOSQUITTO_LIBRARY
    )
        set(Mosquitto_FOUND TRUE)
    endif()
endif()

if(Mosquitto_FOUND)
    cmake_path(GET MOSQUITTO_HEADER PARENT_PATH MOSQUITTO_INCLUDE_DIRECTORY)

    add_library(Mosquitto::Mosquitto SHARED IMPORTED)

    if(UNIX)
        set_target_properties(
            Mosquitto::Mosquitto PROPERTIES IMPORTED_LOCATION "${MOSQUITTO_LIBRARY}" INTERFACE_INCLUDE_DIRECTORIES
                                                                                     "${MOSQUITTO_INCLUDE_DIRECTORY}"
        )
    endif()

    if(WIN32)
        set_target_properties(
            Mosquitto::Mosquitto
            PROPERTIES IMPORTED_IMPLIB "${MOSQUITTO_LIBRARY}"
                       IMPORTED_LOCATION "${MOSQUITTO_DLL}"
                       INTERFACE_INCLUDE_DIRECTORIES "${MOSQUITTO_INCLUDE_DIRECTORY}"
        )
    endif()
endif()
