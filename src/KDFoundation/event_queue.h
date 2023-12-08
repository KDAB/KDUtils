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

#include <memory>
#include <mutex>
#include <queue>

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
        m_events.push(std::move(event));
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
        m_events.pop();
        return std::move(ev);
    }

    PostedEvent *peek() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_events.empty())
            return nullptr;
        return m_events.front().get();
    }

    using size_type = std::queue<std::unique_ptr<PostedEvent>>::size_type;
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
    std::queue<std::unique_ptr<PostedEvent>> m_events;
};

} // namespace KDFoundation
