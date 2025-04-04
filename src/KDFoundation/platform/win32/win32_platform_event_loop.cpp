/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/win32/win32_platform_event_loop.h>
#include <KDFoundation/platform/win32/win32_platform_timer.h>
#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/event.h>
#include <KDFoundation/postman.h>

#include <KDUtils/logging.h>

#include <cassert>

using namespace KDFoundation;

namespace {
auto s_msgWindowClassName = L"Win32PlatformEventLoop_msgWindow";

constexpr UINT WM_KD_SOCKETEVENT = WM_USER;
} // namespace

LRESULT CALLBACK Win32PlatformEventLoop::messageWindowProc(HWND hwnd, UINT msgId, WPARAM wp, LPARAM lp)
{
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    auto *loop = reinterpret_cast<Win32PlatformEventLoop *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (msgId == WM_KD_SOCKETEVENT) {
        loop->handleSocketMessage(wp, lp);
        return 0;
    }

    return DefWindowProc(hwnd, msgId, wp, lp);
}

Win32PlatformEventLoop::Win32PlatformEventLoop()
{
    m_wakeUpEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!m_wakeUpEvent)
        SPDLOG_CRITICAL("Failed to create wake up event");

    WNDCLASS wc = {};
    wc.lpfnWndProc = messageWindowProc;
    wc.lpszClassName = s_msgWindowClassName;

    const ATOM atom = RegisterClass(&wc);
    if (!atom)
        SPDLOG_CRITICAL("Failed to register message window class");

    // The GetLastError() returns error code 6 ("The handle is invalid") here but
    // the created message window is fine and works anyway. It's like that even
    // in Microsoft's official examples. Fear not.
    m_msgWindow = CreateWindow(
            s_msgWindowClassName,
            nullptr, 0, 0, 0, 0, 0,
            HWND_MESSAGE,
            nullptr,
            GetModuleHandle(nullptr),
            nullptr);

    if (!m_msgWindow)
        SPDLOG_CRITICAL("Failed to create window for socket events");
    SetWindowLongPtr(m_msgWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

Win32PlatformEventLoop::~Win32PlatformEventLoop()
{
    if (m_wakeUpEvent)
        CloseHandle(m_wakeUpEvent);
    if (m_msgWindow)
        DestroyWindow(m_msgWindow);
    if (!UnregisterClass(s_msgWindowClassName, GetModuleHandle(nullptr)))
        SPDLOG_WARN("Failed to unregister message window class");
}

void Win32PlatformEventLoop::waitForEventsImpl(int timeout)
{
    MSG msg;
    bool hasMessage = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
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
            hasMessage = PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
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
            SPDLOG_CRITICAL("Failed to signal wake up event");
    }
}

bool Win32PlatformEventLoop::registerNotifier(FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;

    if (notifier->fileDescriptor() < 0) {
        SPDLOG_WARN("Attempting to register invalid notifier socket = {}", notifier->fileDescriptor());
        return false;
    }

    NotifierSet &notifierSet = m_notifiers[notifier->fileDescriptor()];
    const NotifierSet backup = notifierSet; // To restore state in case the registration fails

    const auto eventTypeIdx = static_cast<size_t>(notifier->type());

    // Check if already registered for this type
    if (notifierSet.events[eventTypeIdx] != nullptr)
        return false;

    notifierSet.events[eventTypeIdx] = notifier;

    const bool registered = registerWithWSAAsyncSelect(notifier->fileDescriptor(), notifierSet);
    if (!registered) {
        // registration failed. Restore previous state and clean up if needed
        registerWithWSAAsyncSelect(notifier->fileDescriptor(), backup);
        notifierSet = backup;
        if (notifierSet.isEmpty())
            m_notifiers.erase(notifier->fileDescriptor());
    }

    return registered;
}

bool Win32PlatformEventLoop::unregisterNotifier(FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;

    const auto eventTypeIdx = static_cast<size_t>(notifier->type());
    auto &notifierSet = m_notifiers[notifier->fileDescriptor()];

    if (!notifierSet.events[eventTypeIdx]) {
        return false;
    }

    notifierSet.events[eventTypeIdx] = nullptr;

    registerWithWSAAsyncSelect(notifier->fileDescriptor(), notifierSet);

    if (notifierSet.isEmpty())
        m_notifiers.erase(notifier->fileDescriptor());

    return true;
}

std::unique_ptr<AbstractPlatformTimer> Win32PlatformEventLoop::createPlatformTimerImpl(Timer *timer)
{
    return std::make_unique<Win32PlatformTimer>(timer);
}

void Win32PlatformEventLoop::handleSocketMessage(WPARAM wparam, LPARAM lparam)
{
    const int sockId = static_cast<int>(wparam);
    const int err = WSAGETSELECTERROR(lparam);
    const int op = WSAGETSELECTEVENT(lparam);

    if (err) {
        // TODO: Can we deliver the message to the notifier too? For e.g. if we get a
        // WSAECONNREFUSED error, we should be able to deliver it to the notifier.
        // See winerror.h for the error codes.
        SPDLOG_WARN("Error when receiving message on socket {}, Err = {}", sockId, err);
        return;
    }

    if (!m_postman) {
        SPDLOG_WARN("No postman set. Cannot deliver events");
        return;
    }

    const auto &notifierSet = m_notifiers[sockId];

    switch (op) {
    case FD_READ:
    case FD_CLOSE:
    case FD_ACCEPT:
        if (notifierSet.events[0]) { // read event
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.events[0], &ev);
        }
        break;
    case FD_WRITE:
    case FD_CONNECT:
        if (notifierSet.events[1]) { // write event
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.events[1], &ev);
        }
        break;
    case FD_OOB:
        if (notifierSet.events[2]) { // exception event
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.events[2], &ev);
        }
    default:
        break;
    }
}

bool KDFoundation::Win32PlatformEventLoop::registerWithWSAAsyncSelect(int fd, const NotifierSet &notifiers) const
{
    auto typeToWsaEvents = [](FileDescriptorNotifier::NotificationType type) {
        switch (type) {
        case FileDescriptorNotifier::NotificationType::Read:
            return FD_READ | FD_CLOSE | FD_ACCEPT;
        case FileDescriptorNotifier::NotificationType::Write:
            return FD_WRITE | FD_CONNECT;
        case FileDescriptorNotifier::NotificationType::Exception:
            return FD_OOB;
        }
        return 0;
    };

    // If all notifiers are empty, we'll pass zero and unregister completely for this fd
    long int eventsToSubscribe = 0;
    const std::array<FileDescriptorNotifier::NotificationType, 3> types = {
        FileDescriptorNotifier::NotificationType::Read,
        FileDescriptorNotifier::NotificationType::Write,
        FileDescriptorNotifier::NotificationType::Exception,
    };
    for (auto type : types) {
        if (notifiers.events[static_cast<size_t>(type)])
            eventsToSubscribe |= typeToWsaEvents(type);
    }

    const int result = WSAAsyncSelect(fd, m_msgWindow, WM_KD_SOCKETEVENT, eventsToSubscribe);
    if (result != 0) {
        SPDLOG_ERROR("Failed to register notifier for socket = {}. Error = {}", fd, result);
        return false;
    }

    return true;
}
