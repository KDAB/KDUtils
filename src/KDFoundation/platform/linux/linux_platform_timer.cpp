/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_timer.h>

#include <unistd.h>
#include <sys/timerfd.h>

#include "KDFoundation/core_application.h"
#include "KDFoundation/timer.h"
#include "KDFoundation/platform/linux/linux_platform_event_loop.h"

using namespace KDFoundation;

LinuxPlatformTimer::LinuxPlatformTimer(Timer *timer)
    : m_notifier(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC), FileDescriptorNotifier::NotificationType::Read)
{
    m_notifier.triggered.connect([this, timer]() {
        char buf[8];
        const auto bytes = read(m_notifier.fileDescriptor(), buf, 8);
        KD_UNUSED(bytes);
        timer->timeout.emit();
    });

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
    itimerspec spec = {
        .it_interval = time,
        .it_value = time
    };
    timerfd_settime(m_notifier.fileDescriptor(), 0, &spec, nullptr);
}

void LinuxPlatformTimer::disarm()
{
    timespec time = { 0, 0 };
    itimerspec spec = {
        .it_interval = time,
        .it_value = time
    };
    timerfd_settime(m_notifier.fileDescriptor(), 0, &spec, nullptr);
}
