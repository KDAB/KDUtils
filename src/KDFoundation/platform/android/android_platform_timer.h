/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>

#include <kdbindings/signal.h>

#include <KDFoundation/platform/abstract_platform_timer.h>
#include <KDFoundation/file_descriptor_notifier.h>

namespace KDFoundation {

class Timer;

class KDFOUNDATION_API AndroidPlatformTimer : public AbstractPlatformTimer
{
public:
    AndroidPlatformTimer(Timer *timer);
    ~AndroidPlatformTimer() override;

private:
    void arm(std::chrono::microseconds us);
    void disarm();

    struct FdCloser {
        ~FdCloser();
        int fd;
    } m_fdCloser;
    FileDescriptorNotifier m_notifier;
    KDBindings::ScopedConnection m_notifierConnection;
    KDBindings::ScopedConnection m_timerRunningConnection;
    KDBindings::ScopedConnection m_timerIntervalConnection;
};

} // namespace KDFoundation
