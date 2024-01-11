/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_event_loop.h>
#include <KDFoundation/platform/linux/linux_platform_timer.h>
#include <KDFoundation/event.h>
#include <KDFoundation/postman.h>

#include <unistd.h>
#include <sys/eventfd.h>

using namespace KDFoundation;

LinuxPlatformEventLoop::LinuxPlatformEventLoop()
    : AbstractPlatformEventLoop()
{
    // We use epoll to multiplex as it has much better performance than
    // select or poll.
    m_epollHandle = epoll_create1(0);
    if (m_epollHandle == -1)
        SPDLOG_CRITICAL("Failed to initialise epoll");
    SPDLOG_DEBUG("Initialised epoll instance");

    // We use eventfd as a way to wake up the event loop from another thread.
    // E.g. after posting an event to the event queue eventfd can be used to
    // wake up the waitForEvents() function which will then allow the event
    // queue to be processed.
    m_eventfd = eventfd(0, EFD_NONBLOCK);
    if (m_eventfd == -1)
        SPDLOG_CRITICAL("Failed to initialise eventfd");
    SPDLOG_DEBUG("Initialised eventfd instance");

    // Let's watch the eventfd fd for activity
    if (m_epollHandle && m_eventfd) {
        epoll_event ev = { 0 };
        ev.events = EPOLLIN | EPOLLET; // We only want to know when the status changes, hence Edge Triggered
        ev.data.fd = m_eventfd;
        const int epollOp = EPOLL_CTL_ADD;
        if (epoll_ctl(m_epollHandle, epollOp, m_eventfd, &ev)) {
            SPDLOG_CRITICAL("Failed to register eventfd file descriptor. Error = {}", errno);
            return;
        }

        SPDLOG_DEBUG("Registered eventfd file descriptor");
    }
}

LinuxPlatformEventLoop::~LinuxPlatformEventLoop()
{
    if (close(m_eventfd))
        SPDLOG_CRITICAL("Failed to cleanup eventfd");
    if (close(m_epollHandle))
        SPDLOG_CRITICAL("Failed to cleanup epoll");
}

void LinuxPlatformEventLoop::waitForEvents(int timeout)
{
    const int maxEventCount = 16;
    epoll_event events[maxEventCount];

    int eventCount = epoll_wait(m_epollHandle, events, maxEventCount, timeout);
    SPDLOG_DEBUG("epoll_wait() returned {} events within {} msecs", eventCount, timeout);

    // Let interested parties know if something happened.
    if (!m_postman) {
        SPDLOG_WARN("No postman set. Cannot deliver events");
        return;
    }

    for (int i = 0; i < eventCount; ++i) {
        const auto &ePollEvent = events[i];
        if (ePollEvent.data.fd == m_eventfd)
            continue; // Just our wake up event

        const auto &notifierSet = m_notifiers[ePollEvent.data.fd];

        // Find which notifiers for this fd should be poked
        const uint32_t eventTypes = ePollEvent.events;

        if (notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Read) &&
            (eventTypes & EPOLLIN || eventTypes & EPOLLHUP || eventTypes & EPOLLERR)) {
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.getNotifier(FileDescriptorNotifier::NotificationType::Read), &ev);
        }

        if (notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Write) &&
            (eventTypes & EPOLLOUT || eventTypes & EPOLLHUP || eventTypes & EPOLLERR)) {
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.getNotifier(FileDescriptorNotifier::NotificationType::Write), &ev);
        }

        if (notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Exception) &&
            (eventTypes & EPOLLPRI || eventTypes & EPOLLHUP || eventTypes & EPOLLERR)) {
            NotifierEvent ev;
            m_postman->deliverEvent(notifierSet.getNotifier(FileDescriptorNotifier::NotificationType::Exception), &ev);
        }
    }
}

void LinuxPlatformEventLoop::wakeUp()
{
    eventfd_t value{ 1 };
    eventfd_write(m_eventfd, value);
}

bool LinuxPlatformEventLoop::registerNotifier(FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;

    // Is this file descriptor already being watched?
    auto &notifierSet = m_notifiers[notifier->fileDescriptor()];
    const auto type = notifier->type();
    if (notifierSet.hasNotifier(type))
        return false;

    const bool result = registerFileDescriptor(notifier->fileDescriptor(), type);
    if (result)
        notifierSet.setNotifier(type, notifier);
    return result;
}

bool LinuxPlatformEventLoop::unregisterNotifier(FileDescriptorNotifier *notifier)
{
    if (!notifier)
        return false;

    const auto type = notifier->type();
    const bool result = unregisterFileDescriptor(notifier->fileDescriptor(), type);
    if (result) {
        auto &notifierSet = m_notifiers[notifier->fileDescriptor()];
        notifierSet.resetNotifier(type);
        if (notifierSet.isEmpty())
            m_notifiers.erase(notifier->fileDescriptor());
    }
    return result;
}

bool LinuxPlatformEventLoop::registerFileDescriptor(int fd, FileDescriptorNotifier::NotificationType type)
{
    epoll_event ev = { 0 };
    ev.events = epollEventFromFdPlusType(fd, type);
    ev.data.fd = fd;

    const int epollOp = m_notifiers[fd].isEmpty() ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if (epoll_ctl(m_epollHandle, epollOp, fd, &ev)) {
        SPDLOG_ERROR("Failed to register file descriptor {}. Error = {}", fd, errno);
        return false;
    }

    SPDLOG_DEBUG("Registered file descriptor {}", fd);
    return true;
}

bool LinuxPlatformEventLoop::unregisterFileDescriptor(int fd, FileDescriptorNotifier::NotificationType type)
{
    epoll_event ev;
    ev.events = epollEventFromFdMinusType(fd, type);
    ev.data.fd = fd;

    const int epollOp = m_notifiers[fd].wouldBeEmptyIfUnset(type) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    const auto rv = epoll_ctl(m_epollHandle, epollOp, fd, &ev);

    // In case we attempted to unregister a previously registered file descriptor
    // while the respective file is not accessible any more (resulting in errno == EBADF),
    // proceed as if unregistering the file descriptor was successful.
    // This way we ensure notifiers are reset / deleted properly.
    if (rv && (errno != EBADF)) {
        SPDLOG_ERROR("Failed to unregister file descriptor {}. Error {}: {}.", fd, errno, strerror(errno));
        return false;
    }

    SPDLOG_DEBUG("Unregistered file descriptor {}", fd);
    return true;
}

int LinuxPlatformEventLoop::epollEventFromFdPlusType(int fd, FileDescriptorNotifier::NotificationType type)
{
    const auto &notifierSet = m_notifiers[fd];

    const auto hasReadNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Read);
    const auto hasWriteNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Write);
    const auto hasExceptionNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Exception);

    switch (type) {
    case FileDescriptorNotifier::NotificationType::Read:
        return epollEventFromNotifierTypes(true, hasWriteNotifier, hasExceptionNotifier);
    case FileDescriptorNotifier::NotificationType::Write:
        return epollEventFromNotifierTypes(hasReadNotifier, true, hasExceptionNotifier);
    case FileDescriptorNotifier::NotificationType::Exception:
        return epollEventFromNotifierTypes(hasReadNotifier, hasWriteNotifier, true);
    }
    SPDLOG_CRITICAL("Error in calculating epoll event type");
    return 0;
}

int LinuxPlatformEventLoop::epollEventFromFdMinusType(int fd, FileDescriptorNotifier::NotificationType type)
{
    const auto &notifierSet = m_notifiers[fd];

    const auto hasReadNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Read);
    const auto hasWriteNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Write);
    const auto hasExceptionNotifier = notifierSet.hasNotifier(FileDescriptorNotifier::NotificationType::Exception);

    switch (type) {
    case FileDescriptorNotifier::NotificationType::Read:
        return epollEventFromNotifierTypes(false, hasWriteNotifier, hasExceptionNotifier);
    case FileDescriptorNotifier::NotificationType::Write:
        return epollEventFromNotifierTypes(hasReadNotifier, false, hasExceptionNotifier);
    case FileDescriptorNotifier::NotificationType::Exception:
        return epollEventFromNotifierTypes(hasReadNotifier, hasWriteNotifier, false);
    }
    SPDLOG_CRITICAL("Error in calculating epoll event type");
    return 0;
}

int LinuxPlatformEventLoop::epollEventFromNotifierTypes(bool read, bool write, bool exception)
{
    // Epoll automatically listens for EPOLLERR and EPOLLHUP.
    int epollEvent = 0;
    if (read)
        epollEvent |= EPOLLIN;
    if (write)
        epollEvent |= EPOLLOUT;
    if (exception)
        epollEvent |= EPOLLPRI;
    return epollEvent;
}

std::unique_ptr<AbstractPlatformTimer> LinuxPlatformEventLoop::createPlatformTimerImpl(Timer *timer)
{
    return std::make_unique<LinuxPlatformTimer>(timer);
}
