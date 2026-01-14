/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>
#include <windows.h>
#include <kdbindings/signal.h>
#undef max

#include <KDFoundation/platform/abstract_platform_timer.h>

namespace KDFoundation {

class Timer;

class KDFOUNDATION_API Win32PlatformTimer : public AbstractPlatformTimer
{
public:
    Win32PlatformTimer(Timer *timer);
    ~Win32PlatformTimer() override;

private:
    void arm(std::chrono::microseconds us);
    void disarm();
    static void CALLBACK callback(HWND hwnd, UINT uMsg, UINT_PTR timerId, DWORD dwTime);

    Timer *m_timer;
    KDBindings::ScopedConnection m_timerRunningConnection;
    KDBindings::ScopedConnection m_timerIntervalConnection;
    uintptr_t m_id{ 0 };
};

} // namespace KDFoundation
