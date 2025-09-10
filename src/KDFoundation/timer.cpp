/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "timer.h"

#include "core_application.h"
#include "event_loop.h"
#include "platform/abstract_platform_timer.h"

namespace KDFoundation {

// Initialize the static atomic counter
std::atomic<Timer::TimerId> Timer::s_nextId = 1;

namespace {
std::unique_ptr<AbstractPlatformTimer> createPlatformTimer(Timer *instance)
{
    auto eventLoop = EventLoop::instance();
    assert(eventLoop && "Current thread must have an event loop. Create an instance of KDFoundation::EventLoop on the local thread to use a timer.");
    return eventLoop->platformEventLoop()->createPlatformTimer(instance);
}
} // namespace

Timer::Timer()
    : m_platformTimer(createPlatformTimer(this))
{
}

Timer::~Timer()
{
}

void Timer::handleTimeout()
{
    // If this is a single shot timer, stop it before emitting the timeout signal
    if (singleShot())
        running = false;

    // Emit the timeout signal
    timeout.emit();
}

std::map<Timer::TimerId, Timer::TimerEntry> &Timer::getManagedTimers()
{
    static std::map<TimerId, TimerEntry> timerMap;
    return timerMap;
}

bool Timer::cancelTimer(TimerId id)
{
    auto &timerMap = getManagedTimers();
    auto it = timerMap.find(id);
    if (it != timerMap.end()) {
        it->second.timer->running = false;
        timerMap.erase(it);
        return true;
    }
    return false;
}

void Timer::cancelAllTimers()
{
    auto &timerMap = getManagedTimers();
    for (auto &pair : timerMap) {
        pair.second.timer->running = false;
    }
    timerMap.clear();
}

bool Timer::isTimerActive(TimerId id)
{
    auto &timerMap = getManagedTimers();
    auto it = timerMap.find(id);
    return (it != timerMap.end() && it->second.timer->running());
}

} // namespace KDFoundation
