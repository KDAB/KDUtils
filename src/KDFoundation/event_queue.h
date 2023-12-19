/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/event.h>

#include <algorithm>
#include <memory>
#include <mutex>
#include <deque>

namespace KDFoundation {

class KDFOUNDATION_API EventQueue
{
public:
    EventQueue()
    {
    }

    void push(std::unique_ptr<PostedEvent> &&event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_events.push_back(std::move(event));
    }

    void push(EventReceiver *target, std::unique_ptr<Event> &&event)
    {
        auto ev = std::make_unique<PostedEvent>(target, std::forward<std::unique_ptr<Event>>(event));
        push(std::move(ev));
    }

    std::unique_ptr<PostedEvent> tryPop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_events.empty())
            return std::unique_ptr<PostedEvent>();
        auto ev = std::move(m_events.front());
        m_events.pop_front();
        return std::move(ev);
    }

    PostedEvent *peek() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_events.empty())
            return nullptr;
        return m_events.front().get();
    }

    void removeAllEventsTargeting(EventReceiver &eventReceiver)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto postedEventTargetsEventReceiver = [&eventReceiver](std::unique_ptr<PostedEvent> &postedEvent) {
            return (postedEvent->target() == &eventReceiver);
        };
        const auto it = std::remove_if(m_events.begin(), m_events.end(), postedEventTargetsEventReceiver);
        m_events.erase(it, m_events.end());
    }

    using size_type = std::deque<std::unique_ptr<PostedEvent>>::size_type;
    size_type size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_events.size();
    }

    bool isEmpty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_events.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::deque<std::unique_ptr<PostedEvent>> m_events;
};

} // namespace KDFoundation
