/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/win32/win32_platform_event_loop.h>
#include <KDFoundation/platform/win32/win32_platform_timer.h>

#include <KDFoundation/logging.h>

#include <assert.h>

using namespace KDFoundation;

Win32PlatformEventLoop::Win32PlatformEventLoop()
{
    m_wakeUpEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!m_wakeUpEvent)
        spdlog::critical("Failed to create wake up event");
}

Win32PlatformEventLoop::~Win32PlatformEventLoop()
{
    if (m_wakeUpEvent)
        CloseHandle(m_wakeUpEvent);
}

void Win32PlatformEventLoop::waitForEvents(int timeout)
{
    MSG msg;
    bool hasMessage = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    if (!hasMessage) {
        // sleep until we get a message or the wake up event is signaled
        DWORD nCount = 0;
        HANDLE *pHandles = nullptr;
        if (m_wakeUpEvent) {
            nCount = 1;
            pHandles = &m_wakeUpEvent;
        }
        const DWORD dwTimeout = timeout == -1 ? INFINITE : timeout;
        const auto waitRet = MsgWaitForMultipleObjects(nCount, pHandles, FALSE, dwTimeout, QS_ALLINPUT);
        if (waitRet == WAIT_OBJECT_0) {
            // wake up event was signaled
            assert(m_wakeUpEvent);
            ResetEvent(m_wakeUpEvent);
        } else {
            // either there's a message in the input queue or we timed out
            hasMessage = PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
        }
    }
    if (hasMessage) {
        if (msg.message == WM_QUIT) {
            // TODO: close all windows
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Win32PlatformEventLoop::wakeUp()
{
    if (m_wakeUpEvent) {
        if (!SetEvent(m_wakeUpEvent))
            spdlog::critical("Failed to signal wake up event");
    }
}

bool Win32PlatformEventLoop::registerNotifier(FileDescriptorNotifier * /* notifier */)
{
    // TODO
    return false;
}

bool Win32PlatformEventLoop::unregisterNotifier(FileDescriptorNotifier * /* notifier */)
{
    // TODO
    return false;
}

std::unique_ptr<AbstractPlatformTimer> Win32PlatformEventLoop::createPlatformTimerImpl(Timer *timer)
{
    return std::make_unique<Win32PlatformTimer>(timer);
}
