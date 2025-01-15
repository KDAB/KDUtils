/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "macos_platform_event_loop.h"
#include "macos_platform_timer.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <limits>
#include <memory>

constexpr auto KDFoundationCocoaEventSubTypeWakeup = std::numeric_limits<short>::max();

namespace KDFoundation {

MacOSPlatformEventLoop::MacOSPlatformEventLoop()
{
    @autoreleasepool {
        // make sure there's a NSApp
        [NSApplication sharedApplication];
    }
}

MacOSPlatformEventLoop::~MacOSPlatformEventLoop() = default;

void MacOSPlatformEventLoop::waitForEventsImpl(int timeout)
{
    @autoreleasepool {
        NSDate *expiration = [timeout] {
            if (timeout == -1)
                return [NSDate distantFuture];
            if (timeout == 0)
                return [NSDate distantPast];
            return [NSDate dateWithTimeIntervalSinceNow:static_cast<double>(timeout) / 1000.0];
        }();
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:expiration inMode:NSDefaultRunLoopMode dequeue:YES];
        if (event)
            [NSApp sendEvent:event];
    }
}

void MacOSPlatformEventLoop::wakeUp()
{
    // post a dummy event to wake up the event loop
    postEmptyEvent();
}

void MacOSPlatformEventLoop::postEmptyEvent()
{
    @autoreleasepool {
        [NSApp postEvent:[NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                            location:NSZeroPoint
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0
                                             context:nil
                                             subtype:KDFoundationCocoaEventSubTypeWakeup
                                               data1:0
                                               data2:0]
                 atStart:NO];
    }
}

bool MacOSPlatformEventLoop::registerNotifier(FileDescriptorNotifier * /* notifier */)
{
    // TODO
    return false;
}

bool MacOSPlatformEventLoop::unregisterNotifier(FileDescriptorNotifier * /* notifier */)
{
    // TODO
    return false;
}

std::unique_ptr<AbstractPlatformTimer> MacOSPlatformEventLoop::createPlatformTimerImpl(Timer *timer)
{
    return std::make_unique<MacOSPlatformTimer>(timer);
}

} // namespace KDFoundation
