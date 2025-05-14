# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(open_ai_client_sse LANGUAGES CXX)

add_executable(
    ${PROJECT_NAME}
    main.cpp open_ai_client_sse.cpp open_ai_client_sse.h
)

target_link_libraries(
    ${PROJECT_NAME} KDUtils::KDNetwork nlohmann_json::nlohmann_json
)
