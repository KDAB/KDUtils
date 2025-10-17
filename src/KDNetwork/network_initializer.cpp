/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "network_initializer.h"

#include <KDFoundation/config.h>

#include <KDUtils/logging.h>

#if defined(KD_PLATFORM_WIN32)
#include <winsock2.h>
#endif

namespace KDNetwork {

NetworkInitializer &NetworkInitializer::instance()
{
    static NetworkInitializer instance;
    return instance;
}

NetworkInitializer::NetworkInitializer()
{
#if defined(KD_PLATFORM_WIN32)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        KDUtils::Logger::logger("KDNetwork")->error("WSAStartup failed: {}", result);
    } else {
        KDUtils::Logger::logger("KDNetwork")->debug("Network subsystem initialized");
    }
#endif
}

NetworkInitializer::~NetworkInitializer()
{
#if defined(KD_PLATFORM_WIN32)
    WSACleanup();
    KDUtils::Logger::logger("KDNetwork")->debug("Network subsystem cleaned up");
#endif
}

} // namespace KDNetwork
