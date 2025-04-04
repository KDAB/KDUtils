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
    if (m_isEnabled)
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
    // TODO: Handle this on other threads by getting the event loop from
    //       some thread local storage.
    // Get hold of the event loop from the application
    auto app = CoreApplication::instance();
    if (!app) {
        SPDLOG_WARN("No application object exists yet. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    auto eventLoop = app->eventLoop();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists yet. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    const bool result = eventLoop->registerNotifier(this);
    if (!result)
        SPDLOG_WARN("Failed to register notifier for fd {} with the event loop", m_fd);
}

void KDFoundation::FileDescriptorNotifier::unregisterNotifier()
{
    auto app = CoreApplication::instance();
    if (!app) {
        SPDLOG_WARN("No application object exists yet we still have a notifier for fd {} alive", m_fd);
        return;
    }

    auto eventLoop = app->eventLoop();
    if (!eventLoop) {
        SPDLOG_WARN("No event loop exists yet. The notifier for fd {} will not be unregistered", m_fd);
        return;
    }

    // Unregister the notifier from the event loop
    const bool result = eventLoop->unregisterNotifier(this);
    if (!result)
        SPDLOG_WARN("Failed to unregister notifier for fd {} from the event loop", m_fd);
}
