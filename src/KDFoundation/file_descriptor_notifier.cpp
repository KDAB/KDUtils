/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/file_descriptor_notifier.h>
#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/core_application.h>

#include <KDFoundation/logging.h>

#include <cassert>

using namespace KDFoundation;

FileDescriptorNotifier::FileDescriptorNotifier(int fd, NotificationType type)
    : Object()
    , m_fd{ fd }
    , m_type{ type }
{
    assert(m_fd >= 0);

    // TODO: Handle this on other threads by getting the event loop from
    //       some thread local storage.
    // Get hold of the event loop from the application
    auto app = CoreApplication::instance();
    if (!app) {
        spdlog::warn("No application object exists yet. The notifier for fd {} will not be registered", m_fd);
        return;
    }

    auto eventLoop = app->eventLoop();
    if (eventLoop)
        eventLoop->registerNotifier(this);
}

FileDescriptorNotifier::~FileDescriptorNotifier()
{
    auto app = CoreApplication::instance();
    if (!app) {
        spdlog::warn("No application object exists yet we still have a notifier for fd {} alive", m_fd);
        return;
    }

    auto eventLoop = app->eventLoop();
    if (eventLoop)
        eventLoop->unregisterNotifier(this);
}

void FileDescriptorNotifier::event(EventReceiver *target, Event *ev)
{
    if (ev->type() == Event::Type::Notifier) {
        triggered.emit(m_fd);
        ev->setAccepted(true);
    }

    Object::event(target, ev);
}
