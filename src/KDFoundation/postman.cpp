/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "postman.h"
#include "object.h"

#include <KDFoundation/event.h>

using namespace KDFoundation;

void Postman::deliverEvent(Object *target, Event *event)
{
    // Give the registered event filters first chance to handle the event
    for (const auto &filter : m_filters) {
        filter->event(target, event);
        if (event->isAccepted())
            return;
    }

    // TODO: Add support for trickle-down and bubble-up based upon event type
    target->event(target, event);
}

void Postman::addFilter(Object *filter)
{
    assert(filter != nullptr);
    m_filters.push_back(filter);
}

void Postman::removeFilter(Object *filter)
{
    auto it = std::find(m_filters.begin(), m_filters.end(), filter);
    if (it != m_filters.end())
        m_filters.erase(it);
}
