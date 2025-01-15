/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/kdfoundation_global.h>

#include <unordered_map>

namespace KDFoundation {

class MacOSPlatformTimer;

class KDFOUNDATION_API MacOSPlatformEventLoop : public AbstractPlatformEventLoop
{
public:
    MacOSPlatformEventLoop();
    ~MacOSPlatformEventLoop() override;

    MacOSPlatformEventLoop(MacOSPlatformEventLoop const &other) = delete;
    MacOSPlatformEventLoop &operator=(MacOSPlatformEventLoop const &other) = delete;
    MacOSPlatformEventLoop(MacOSPlatformEventLoop &&other) = delete;
    MacOSPlatformEventLoop &operator=(MacOSPlatformEventLoop &&other) = delete;

    void wakeUp() override;

    bool registerNotifier(FileDescriptorNotifier *notifier) override;
    bool unregisterNotifier(FileDescriptorNotifier *notifier) override;

    static void postEmptyEvent();

private:
    void waitForEventsImpl(int timeout) override;
    std::unique_ptr<AbstractPlatformTimer> createPlatformTimerImpl(Timer *timer) override;

    std::unordered_map<void *, MacOSPlatformTimer *> timerMap;
    friend class MacOSPlatformTimer;
};

} // namespace KDFoundation
