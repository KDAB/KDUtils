/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_timer.h>

#include "KDFoundation/core_application.h"
#include "KDFoundation/timer.h"
#include "KDFoundation/platform/linux/linux_platform_event_loop.h"

#include <unistd.h>
#include <sys/timerfd.h>

#include <array>
#include <tuple>

using namespace KDFoundation;

LinuxPlatformTimer::LinuxPlatformTimer(Timer *timer)
    : m_notifier(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC), FileDescriptorNotifier::NotificationType::Read)
{
    m_notifierConnection = m_notifier.triggered.connect([this, timer]() {
        std::array<char, 8> buf;
        std::ignore = read(m_notifier.fileDescriptor(), buf.data(), buf.size());
        // Use handleTimeout instead of directly emitting the timeout signal
        timer->handleTimeout();
    });

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

LinuxPlatformTimer::~LinuxPlatformTimer()
{
    // the fd needs to be unregistered before closing it, so we use this fd closer helper
    // to achieve that, since its destructor will be called after ~FileDescriptorNotifier().
    m_fdCloser.fd = m_notifier.fileDescriptor();
}

LinuxPlatformTimer::FdCloser::~FdCloser()
{
    close(fd);
}

void LinuxPlatformTimer::arm(std::chrono::microseconds us)
{
    timespec time;
    time.tv_sec = us.count() / 1'000'000;
    time.tv_nsec = (us.count() - time.tv_sec * 1'000'000) * 1000;
    const itimerspec spec = {
        .it_interval = time,
        .it_value = time
    };
    timerfd_settime(m_notifier.fileDescriptor(), 0, &spec, nullptr);
}

void LinuxPlatformTimer::disarm()
{
    const timespec time = { 0, 0 };
    const itimerspec spec = {
        .it_interval = time,
        .it_value = time
    };
    timerfd_settime(m_notifier.fileDescriptor(), 0, &spec, nullptr);
}
