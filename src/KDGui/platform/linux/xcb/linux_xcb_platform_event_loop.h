/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/linux/linux_platform_event_loop.h>

#include <KDFoundation/logging.h>

#include <memory>

namespace KDFoundation {
class FileDescriptorNotifier;
}

namespace KDGui {

class LinuxXcbPlatformIntegration;
class LinuxXkbKeyboard;

class LinuxXcbPlatformEventLoop : public KDFoundation::LinuxPlatformEventLoop
{
public:
    explicit LinuxXcbPlatformEventLoop(LinuxXcbPlatformIntegration *platformIntegration);
    ~LinuxXcbPlatformEventLoop();

    void waitForEvents(int timeout) override;

private:
    void processXcbEvents();

    std::shared_ptr<spdlog::logger> m_logger;
    LinuxXcbPlatformIntegration *m_platformIntegration;
    LinuxXkbKeyboard *m_keyboard{ nullptr };
    std::unique_ptr<KDFoundation::FileDescriptorNotifier> m_xcbNotifier;
    bool m_xcbEventsPending{ false };
};

} // namespace KDGui
