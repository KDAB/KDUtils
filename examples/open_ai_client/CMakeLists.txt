# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(open_ai_client LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    main.cpp open_ai_client.cpp open_ai_client.h
)

target_link_libraries(
    ${PROJECT_NAME} KDUtils::KDNetwork nlohmann_json::nlohmann_json
)

# On windows, define _CRT_SECURE_NO_WARNINGS to avoid warnings about std::getenv
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
