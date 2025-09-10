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
#include "file_descriptor_notifier.h"

using namespace KDFoundation;

FileDescriptorNotifier::FileDescriptorNotifier(int fd, NotificationType type)
    : m_fd{ fd }
    , m_type{ type }
{
    assert(m_fd >= 0);

    registerNotifier();
}

FileDescriptorNotifier::~FileDescriptorNotifier()
{
    unregisterNotifier();
}

void FileDescriptorNotifier::event(EventReceiver *target, Event *ev)
{
    if (ev->type() == Event::Type::Notifier) {
        triggered.emit(m_fd);
        ev->setAccepted(true);
    }

    Object::event(target, ev);
}

void FileDescriptorNotifier::setEnabled(bool enabled)
{
    if (m_isEnabled == enabled)
        return;

    m_isEnabled = enabled;
    if (m_isEnabled) {
        registerNotifier();
    } else {
        unregisterNotifier();
    }
}

void KDFoundation::FileDescriptorNotifier::registerNotifier()
{
    auto eventLoop = EventLoop::instance();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists on the current thread. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    auto platformEventLoop = eventLoop->platformEventLoop();
    if (!platformEventLoop) {
        SPDLOG_WARN("No platform event loop exists on the current thread. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    const bool result = platformEventLoop->registerNotifier(this);
    if (!result)
        SPDLOG_WARN("Failed to register notifier for fd {} with the event loop", m_fd);
}

void KDFoundation::FileDescriptorNotifier::unregisterNotifier()
{
    auto eventLoop = EventLoop::instance();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists yet. The notifier for fd {} will not be unregistered", m_fd);
        return;
    }

    auto platformEventLoop = eventLoop->platformEventLoop();
    if (!platformEventLoop) {
        SPDLOG_WARN("No platform event loop exists yet. The notifier for fd {} will not be unregistered", m_fd);
        return;
    }

    // Unregister the notifier from the event loop
    const bool result = platformEventLoop->unregisterNotifier(this);
    if (!result)
        SPDLOG_WARN("Failed to unregister notifier for fd {} from the event loop", m_fd);
}
