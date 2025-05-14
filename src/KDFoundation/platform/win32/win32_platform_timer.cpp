/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/win32/win32_platform_timer.h>
#include <KDFoundation/platform/win32/win32_platform_event_loop.h>

#include <cassert>

#include "KDFoundation/core_application.h"
#include "KDFoundation/timer.h"

using namespace KDFoundation;

inline Win32PlatformEventLoop *eventLoop()
{
    return static_cast<Win32PlatformEventLoop *>(CoreApplication::instance()->eventLoop());
}

Win32PlatformTimer::Win32PlatformTimer(Timer *timer)
    : m_timer(timer)
{
    m_timerRunningConnection = timer->running.valueChanged().connect([this, timer](bool running) {
        if (running) {
            arm(timer->interval.get());
        } else {
            disarm();
        }
    });
    m_timerIntervalConnection = timer->interval.valueChanged().connect([this, timer]() {
        if (timer->running.get()) {
            arm(timer->interval.get());
        }
    });
}

Win32PlatformTimer::~Win32PlatformTimer()
{
    if (m_id != 0) {
        disarm();
    }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void Win32PlatformTimer::callback(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR timerId, DWORD /*dwTime*/)
{
    auto timer = eventLoop()->timers[timerId];
    assert(timer);

    // Use handleTimeout instead of directly emitting the timeout signal
    timer->m_timer->handleTimeout();
}

void Win32PlatformTimer::arm(std::chrono::microseconds us)
{
    auto msecs = us.count() / 1000;
    if (m_id == 0) {
        m_id = SetTimer(nullptr, 0, UINT(msecs), callback);
        if (m_id) {
            eventLoop()->timers[m_id] = this;
        }
    } else {
        SetTimer(nullptr, m_id, UINT(msecs), callback);
    }
}

void Win32PlatformTimer::disarm()
{
    assert(m_id != 0);

    eventLoop()->timers.erase(m_id);
    KillTimer(nullptr, m_id);
    m_id = 0;
}
