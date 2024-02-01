/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <unordered_map>
#include <array>

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/kdfoundation_global.h>

#include <windows.h>

namespace KDFoundation {

class FileDescriptorNotifier;
class Win32PlatformTimer;

class KDFOUNDATION_API Win32PlatformEventLoop : public AbstractPlatformEventLoop
{
public:
    Win32PlatformEventLoop();
    ~Win32PlatformEventLoop() override;

    Win32PlatformEventLoop(Win32PlatformEventLoop const &other) = delete;
    Win32PlatformEventLoop &operator=(Win32PlatformEventLoop const &other) = delete;
    Win32PlatformEventLoop(Win32PlatformEventLoop &&other) = delete;
    Win32PlatformEventLoop &operator=(Win32PlatformEventLoop &&other) = delete;

    void waitForEvents(int timeout) override;

    void wakeUp() override;

    bool registerNotifier(FileDescriptorNotifier *notifier) override;
    bool unregisterNotifier(FileDescriptorNotifier *notifier) override;

    std::unordered_map<uintptr_t, Win32PlatformTimer *> timers;

private:
    std::unique_ptr<AbstractPlatformTimer> createPlatformTimerImpl(Timer *timer) override;

    struct NotifierSet {
        std::array<FileDescriptorNotifier *, 3> events{ nullptr, nullptr, nullptr };
        bool isEmpty() const
        {
            return events[0] == nullptr && events[1] == nullptr && events[2] == nullptr;
        }
    };
    std::unordered_map<int, NotifierSet> m_notifiers;

    static LRESULT CALLBACK messageWindowProc(HWND, UINT, WPARAM, LPARAM);
    void handleSocketMessage(WPARAM wParam, LPARAM lParam);
    bool registerWithWSAAsyncSelect(int fd, const NotifierSet &notifiers) const;

    HANDLE m_wakeUpEvent;
    HWND m_msgWindow = 0;
};

} // namespace KDFoundation
