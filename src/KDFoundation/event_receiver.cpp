/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "event_receiver.h"
#include "core_application.h"

using namespace KDFoundation;

EventReceiver::~EventReceiver() noexcept
{
    // Clear this thread's and the main event loop from events targeting this receiver
    // FIXME Events posted to other threads' event loops are not cleared
    if (EventLoop::instance() != nullptr)
        EventLoop::instance()->removeAllEventsTargeting(*this);

    if (CoreApplication::instance() != nullptr && CoreApplication::instance()->eventLoop() != EventLoop::instance())
        CoreApplication::instance()->removeAllEventsTargeting(*this);
}
