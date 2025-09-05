/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/linux/linux_platform_event_loop.h>

#include <KDUtils/logging.h>

#include <memory>

namespace KDGui {

class LinuxWaylandPlatformIntegration;

class LinuxWaylandPlatformEventLoop : public KDFoundation::LinuxPlatformEventLoop
{
public:
    explicit LinuxWaylandPlatformEventLoop(LinuxWaylandPlatformIntegration *platformIntegration);
    ~LinuxWaylandPlatformEventLoop();

    void waitForEventsImpl(int timeout) override;

private:
    std::shared_ptr<spdlog::logger> m_logger;
    LinuxWaylandPlatformIntegration *m_platformIntegration;
};

} // namespace KDGui
