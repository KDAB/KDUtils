/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "macos_platform_timer.h"
#include "macos_platform_event_loop.h"

#include <Foundation/Foundation.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <type_traits>
#include <unistd.h>
#import <AppKit/AppKit.h>

#include "KDFoundation/core_application.h"
#include "KDFoundation/timer.h"
#include "KDFoundation/platform/macos/macos_platform_event_loop.h"

namespace KDFoundation {

class NSTimerWrapper
{
    NSTimer *timer;
};
} // namespace KDFoundation

using namespace KDFoundation;

inline MacOSPlatformEventLoop *eventLoop()
{
    return static_cast<MacOSPlatformEventLoop *>(CoreApplication::instance()->eventLoop());
}

MacOSPlatformTimer::MacOSPlatformTimer(Timer *timer)
    : m_handler{ timer }, highResTimer{ nullptr }
{

    @autoreleasepool {
        // make sure there's a NSApp
        [NSApplication sharedApplication];
    }

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

void MacOSPlatformTimer::timerFired(void *context)
{
    @autoreleasepool {
        MacOSPlatformEventLoop *ev = eventLoop();
        dispatch_source_t caller = (dispatch_source_t)context;
        if (auto it = ev->timerMap.find(caller); it != ev->timerMap.end()) {
            it->second->m_handler->timeout.emit();
        }
    }
}

void MacOSPlatformTimer::arm(std::chrono::microseconds us)
{
  @autoreleasepool {

      if (highResTimer) {
          disarm();
      }
      const auto interval = std::chrono::duration_cast<std::chrono::nanoseconds>(us).count();
      dispatch_queue_main_t mainQueue = dispatch_get_main_queue();
      highResTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, mainQueue);
      dispatch_source_set_timer(highResTimer,
                                dispatch_time(DISPATCH_TIME_NOW, interval),
                                interval,
                                0);
      dispatch_source_set_event_handler_f(highResTimer, timerFired);
      dispatch_set_context(highResTimer, highResTimer);
      dispatch_resume(highResTimer);

      if (highResTimer) {
          void *key = reinterpret_cast<void *>(highResTimer);
          eventLoop()->timerMap[key] = this;
      }
  }
}

void MacOSPlatformTimer::disarm()
{
    @autoreleasepool {
        if (highResTimer) {
            void *key = reinterpret_cast<void *>(highResTimer);
            eventLoop()->timerMap.erase(key);
            dispatch_source_cancel(highResTimer);
            highResTimer = nullptr;
        }
    }
}
