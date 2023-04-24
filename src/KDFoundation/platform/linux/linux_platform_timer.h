/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>

#include <KDFoundation/platform/abstract_platform_timer.h>
#include <KDFoundation/file_descriptor_notifier.h>

namespace KDFoundation {

class Timer;

class KDFOUNDATION_API LinuxPlatformTimer : public AbstractPlatformTimer
{
public:
    LinuxPlatformTimer(Timer *timer);
    ~LinuxPlatformTimer() override;

private:
    void arm(std::chrono::microseconds us);
    void disarm();

    struct FdCloser {
        ~FdCloser();
        int fd;
    } m_fdCloser;
    FileDescriptorNotifier m_notifier;
};

} // namespace KDFoundation
