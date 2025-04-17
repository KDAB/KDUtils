/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <functional>
#include <atomic>

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
    using TimerId = uint64_t;

    explicit Timer();
    ~Timer();

    KDBindings::Signal<> timeout;

    KDBindings::Property<bool> running{ false };
    KDBindings::Property<std::chrono::microseconds> interval{};
    KDBindings::Property<bool> singleShot{ false };

    // Static helper methods for creating timers. We offer two options:
    // 1. Explicit ownership of the timer via unique_ptr
    // 2. JavaScript-style timer management (fire and forget with ID)
    // The first option is more C++ idiomatic, while the second is more JavaScript-like.
    // Both options are available to accommodate different use cases and preferences.

    // Option 1: Explicit ownership of timer via unique_ptr
    template<typename Func>
    static std::unique_ptr<Timer> createTimeout(Func &&callback, std::chrono::milliseconds delay)
    {
        auto timer = std::make_unique<Timer>();
        timer->singleShot = true;
        timer->interval = std::chrono::duration_cast<std::chrono::microseconds>(delay);
        std::ignore = timer->timeout.connect(std::forward<Func>(callback));
        timer->running = true;
        return timer;
    }

    template<typename Func>
    static std::unique_ptr<Timer> createRecurring(Func &&callback, std::chrono::milliseconds interval)
    {
        auto timer = std::make_unique<Timer>();
        timer->singleShot = false;
        timer->interval = std::chrono::duration_cast<std::chrono::microseconds>(interval);
        std::ignore = timer->timeout.connect(std::forward<Func>(callback));
        timer->running = true;
        return timer;
    }

    // Option 2: JavaScript-style timer management (fire and forget with ID)
    template<typename Func>
    static TimerId setTimeout(std::chrono::milliseconds delay, Func &&callback)
    {
        return createManagedTimer(true, delay, std::forward<Func>(callback));
    }

    template<typename Func>
    static TimerId setRecurring(std::chrono::milliseconds interval, Func &&callback)
    {
        return createManagedTimer(false, interval, std::forward<Func>(callback));
    }

    // Cancel a timer created with setTimout() or setRecurring()
    static bool cancelTimer(TimerId id);

    // Cancel all managed timers
    static void cancelAllTimers();

    // Check if a managed timer is active
    static bool isTimerActive(TimerId id);

private:
    // Allow platform-specific timer implementations to call handleTimeout
    friend class Win32PlatformTimer;
    friend class LinuxPlatformTimer;
    friend class MacOSPlatformTimer;

    // Helper method for platform timers to call when timeout occurs
    void handleTimeout();

    std::unique_ptr<AbstractPlatformTimer> m_platformTimer;

    // Next ID for managed timers - static across all function calls
    static std::atomic<TimerId> s_nextId;

    struct TimerEntry {
        std::unique_ptr<Timer> timer;
        KDBindings::ScopedConnection callbackConnection;
        KDBindings::ScopedConnection cleanupConnection;
    };

    // For managed timers (the javascript-style timers)
    template<typename Func>
    static TimerId createManagedTimer(bool singleShot, std::chrono::milliseconds interval, Func &&callback)
    {
        TimerId id = s_nextId++;

        TimerEntry entry{ std::make_unique<Timer>(), {}, {} };
        entry.timer->singleShot = singleShot;
        entry.timer->interval = std::chrono::duration_cast<std::chrono::microseconds>(interval);

        // Connect the user's callback - before the custom slot below to ensure the user's callback is called first
        // before we destroy the timer (and its timeout signal).
        entry.callbackConnection = entry.timer->timeout.connect(std::forward<Func>(callback));

        // For single-shot timers, add a connection to clean up the timer after it fires
        if (singleShot) {
            entry.cleanupConnection = entry.timer->timeout.connect([id]() {
                getManagedTimers().erase(id);
            });
        }

        // Start the timer
        entry.timer->running = true;

        // Store the timer in our managed collection
        getManagedTimers()[id] = std::move(entry);

        return id;
    }

    static std::map<TimerId, TimerEntry> &getManagedTimers();
};

} // namespace KDFoundation
