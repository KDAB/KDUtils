/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/core_application.h>

#include <KDUtils/logging.h>

#include <cassert>

using namespace KDFoundation;

FileDescriptorNotifier::FileDescriptorNotifier(int fd, NotificationType type)
    : m_fd{ fd }
    , m_type{ type }
{
    assert(m_fd >= 0);

    // Get hold of the current thread's event loop
    auto eventLoop = EventLoop::instance();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists on the current thread. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    auto platformEventLoop = eventLoop->platformEventLoop();
    if (platformEventLoop)
        platformEventLoop->registerNotifier(this);
}

FileDescriptorNotifier::~FileDescriptorNotifier()
{
    auto eventLoop = EventLoop::instance();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists on the current thread, yet we still have a notifier for fd {} alive", m_fd);
        return;
    }

    auto platformEventLoop = eventLoop->platformEventLoop();
    if (platformEventLoop)
        platformEventLoop->unregisterNotifier(this);
}

void FileDescriptorNotifier::event(EventReceiver *target, Event *ev)
{
    if (ev->type() == Event::Type::Notifier) {
        triggered.emit(m_fd);
        ev->setAccepted(true);
    }

    Object::event(target, ev);
}
