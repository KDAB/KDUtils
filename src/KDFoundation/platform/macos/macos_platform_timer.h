/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <CoreFoundation/CoreFoundation.h>

#include <chrono>

#include <KDFoundation/platform/abstract_platform_timer.h>
#include <KDFoundation/file_descriptor_notifier.h>

namespace KDFoundation {

class Timer;

class KDFOUNDATION_API MacOSPlatformTimer : public AbstractPlatformTimer
{
public:
    explicit MacOSPlatformTimer(Timer *timer);
    ~MacOSPlatformTimer() override;

private:
    void arm(std::chrono::microseconds us);
    void disarm();
    static void timerFired(CFRunLoopTimerRef timer, void *info);
    Timer *m_handler;
    CFRunLoopTimerRef cfTimer;
};

} // namespace KDFoundation
