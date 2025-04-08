/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "macos_platform_timer.h"
#include "macos_platform_event_loop.h"
#include "KDFoundation/core_application.h"
#include "KDFoundation/timer.h"
#include "KDFoundation/platform/macos/macos_platform_event_loop.h"

#include <Foundation/Foundation.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <type_traits>
#include <unistd.h>

using namespace KDFoundation;

inline MacOSPlatformEventLoop *eventLoop()
{
    return static_cast<MacOSPlatformEventLoop *>(CoreApplication::instance()->eventLoop());
}

MacOSPlatformTimer::MacOSPlatformTimer(Timer *timer)
    : m_handler{ timer }, cfTimer{ nullptr }
{
    timer->running.valueChanged().connect([this, timer](bool running) {
        if (running) {
            arm(timer->interval.get());
        } else {
            disarm();
        }
    });
    timer->interval.valueChanged().connect([this, timer]() {
        if (timer->running.get()) {
            arm(timer->interval.get());
        }
    });
}

MacOSPlatformTimer::~MacOSPlatformTimer()
{
    disarm();
}

void MacOSPlatformTimer::timerFired(CFRunLoopTimerRef timer, void *)
{
    MacOSPlatformEventLoop *ev = eventLoop();
    void *key = timer;
    if (auto it = ev->timerMap.find(key); it != ev->timerMap.end()) {
        // Use handleTimeout instead of directly emitting the timeout signal
        it->second->m_handler->handleTimeout();
    }
}

void MacOSPlatformTimer::arm(std::chrono::microseconds us)
{
    if (cfTimer) {
        disarm();
    }

    CFTimeInterval interval = std::chrono::duration_cast<std::chrono::duration<double>>(us).count();
    CFRunLoopTimerContext timerContext = { 0, nullptr, nullptr, nullptr, nullptr };
    CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + interval;
    // Create the timer
    cfTimer = CFRunLoopTimerCreate(
            kCFAllocatorDefault, // Allocator
            fireDate, // Fire time
            interval, // Interval
            0, // Flags
            0, // Order
            timerFired, // Callback function
            &timerContext // Timer context
    );
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), cfTimer, kCFRunLoopCommonModes);

    if (cfTimer) {
        void *key = reinterpret_cast<void *>(cfTimer);
        eventLoop()->timerMap[key] = this;
    }
}

void MacOSPlatformTimer::disarm()
{
    if (cfTimer) {
        void *key = reinterpret_cast<void *>(cfTimer);
        eventLoop()->timerMap.erase(key);
        CFRelease(cfTimer);
        cfTimer = nullptr;
    }
}
