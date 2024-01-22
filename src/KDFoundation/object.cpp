/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "object.h"
#include "core_application.h"
#include "event.h"

#include <cassert>
#include <algorithm>

using namespace KDFoundation;

Object::Object()
    : EventReceiver()
    , m_parent{ nullptr }
{
}

void Object::deleteLater()
{
    if (CoreApplication::instance() == nullptr) {
        SPDLOG_ERROR("No CoreApplication object to schedule deferred deletion of object with.");
        return;
    }

    if (CoreApplication::instance() == this) {
        SPDLOG_ERROR("Object::deleteLater() was called on CoreApplication. This is not supported and will be ignored.");
        return;
    }

    auto ev = std::make_unique<DeferredDeleteEvent>();
    CoreApplication::instance()->postEvent(this, std::move(ev));
}

Object::~Object()
{
    destroyed.emit(this);

    // Destroy the children in LIFO to be more like the stack
    while (!m_children.empty()) {
        childRemoved.emit(this, m_children.back().get());
        m_children.pop_back();
    }
}

void Object::event(EventReceiver *target, Event *ev)
{
    if (target != this)
        return;

    switch (ev->type()) {
    case Event::Type::DeferredDelete: {
        // delete object and immediately return from event handler
        delete this;
        return;
    }
    case Event::Type::Timer: {
        timerEvent(static_cast<TimerEvent *>(ev));
        break;
    }

    default: {
        if (ev->type() >= Event::Type::UserType)
            userEvent(ev);
        break;
    }
    }
}

void Object::timerEvent(TimerEvent *ev)
{
}

void Object::userEvent(Event *ev)
{
}
