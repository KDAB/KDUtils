# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

message(STATUS "Looking for KDUtils dependencies")

find_package(spdlog REQUIRED)
find_package(KDBindings REQUIRED)
find_package(whereami REQUIRED)
find_package(mio REQUIRED)

if(KDUTILS_BUILD_MQTT_SUPPORT)
    find_package(mosquitto REQUIRED)
endif()
