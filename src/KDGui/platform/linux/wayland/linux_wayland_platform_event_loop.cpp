/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_wayland_platform_event_loop.h"
#include "linux_wayland_platform_integration.h"

#include <wayland-client-core.h>

using namespace KDGui;
using namespace KDFoundation;

LinuxWaylandPlatformEventLoop::LinuxWaylandPlatformEventLoop(LinuxWaylandPlatformIntegration *platformIntegration)
    : LinuxPlatformEventLoop()
    , m_platformIntegration{ platformIntegration }
{
}

void LinuxWaylandPlatformEventLoop::init()
{
    m_logger = m_platformIntegration->logger();

    int fd = wl_display_get_fd(m_platformIntegration->display());
    registerFileDescriptor(fd, FileDescriptorNotifier::NotificationType::Read);
}

LinuxWaylandPlatformEventLoop::~LinuxWaylandPlatformEventLoop()
{
}

void LinuxWaylandPlatformEventLoop::waitForEvents(int timeout)
{
    auto dpy = m_platformIntegration->display();
    auto queue = m_platformIntegration->queue();

    wl_display_flush(dpy);
    while (wl_display_prepare_read_queue(dpy, queue) != 0) {
        wl_display_dispatch_queue_pending(dpy, queue);
    }

    // Call the base class to do the actual multiplexing
    LinuxPlatformEventLoop::waitForEvents(timeout);

    int fd = wl_display_get_fd(dpy);
    if (epollEventFromFdPlusType(fd, FileDescriptorNotifier::NotificationType::Read)) {
        wl_display_read_events(dpy);
        wl_display_dispatch_queue_pending(dpy, queue);
    } else {
        wl_display_cancel_read(dpy);
    }
}
