/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDNetwork/kdnetwork_export.h>

namespace KDNetwork {

/**
 * NetworkInitializer handles platform-specific network initialization.
 *
 * This class ensures that network subsystems (like Winsock on Windows)
 * are properly initialized before use and cleaned up at program exit.
 * It uses a singleton pattern to guarantee initialization happens only once.
 */
class KDNETWORK_EXPORT NetworkInitializer
{
public:
    /**
     * Get the singleton instance, initializing the network subsystem if needed.
     * @return Reference to the singleton instance
     */
    static NetworkInitializer &instance();

    // Non-copyable
    NetworkInitializer(const NetworkInitializer &) = delete;
    NetworkInitializer &operator=(const NetworkInitializer &) = delete;

private:
    // Private constructor to enforce singleton pattern
    NetworkInitializer();
    ~NetworkInitializer();
};

} // namespace KDNetwork
