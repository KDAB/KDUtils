/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>

#include <kdbindings/property.h>

#include <KDFoundation/kdfoundation_global.h>

namespace KDFoundation {

class AbstractPlatformTimer;
// Forward declaration of platform-specific timer implementations
class Win32PlatformTimer;
class LinuxPlatformTimer;
class MacOSPlatformTimer;

class KDFOUNDATION_API Timer
{
public:
    explicit Timer();
    ~Timer();

    KDBindings::Signal<> timeout;

    KDBindings::Property<bool> running{ false };
    KDBindings::Property<std::chrono::microseconds> interval{};
    KDBindings::Property<bool> singleShot{ false };

private:
    // Allow platform-specific timer implementations to call handleTimeout
    friend class Win32PlatformTimer;
    friend class LinuxPlatformTimer;
    friend class MacOSPlatformTimer;

    // Helper method for platform timers to call when timeout occurs
    void handleTimeout();

    std::unique_ptr<AbstractPlatformTimer> m_platformTimer;
};

} // namespace KDFoundation
