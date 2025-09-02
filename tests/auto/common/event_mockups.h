/*
 * This file is part of KDUtils.
 *
 * SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 * Author: Paul Lemire <paul.lemire@kdab.com>
 * Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Contact KDAB at <info@kdab.com> for commercial licensing options.
 */

#pragma once

#include <KDFoundation/event.h>
#include <KDFoundation/event_loop.h>
#include <KDFoundation/event_receiver.h>
#include <KDFoundation/object.h>

#include <doctest.h>

class PayloadEvent : public KDFoundation::Event
{
public:
    PayloadEvent(int x, int y)
        : Event(static_cast<KDFoundation::Event::Type>(static_cast<uint16_t>(KDFoundation::Event::Type::UserType) + 2))
        , m_x{ x }
        , m_y{ y }
    {
    }

    int m_x;
    int m_y;
};

class CallbackEvent : public KDFoundation::Event
{
public:
    CallbackEvent(const std::function<void()> &callback)
        : Event(static_cast<KDFoundation::Event::Type>(static_cast<uint16_t>(KDFoundation::Event::Type::UserType) + 1))
        , callback(callback)
    {
    }

    std::function<void()> callback;
};

class EventObject : public KDFoundation::Object
{
public:
    EventObject(int expectedX, int expectedY)
        : m_expectedX{ expectedX }
        , m_expectedY{ expectedY }
    {
    }

    bool userEventDelivered() const { return m_userEventDelivered; }

protected:
    void userEvent(KDFoundation::Event *ev) override
    {
        if (ev->type() == static_cast<KDFoundation::Event::Type>(static_cast<uint16_t>(KDFoundation::Event::Type::UserType) + 1)) {
            auto e = static_cast<CallbackEvent *>(ev);
            e->setAccepted(true);
            e->callback();
        }
        if (ev->type() == static_cast<KDFoundation::Event::Type>(static_cast<uint16_t>(KDFoundation::Event::Type::UserType) + 2)) {
            auto e = static_cast<PayloadEvent *>(ev);
            if (e->m_x == m_expectedX && e->m_y == m_expectedY) {
                e->setAccepted(true);
                m_userEventDelivered = true;
            }
        }
    }

private:
    int m_expectedX;
    int m_expectedY;
    bool m_userEventDelivered = false;
};

class RecursiveEventPosterObject : public KDFoundation::Object
{
public:
    RecursiveEventPosterObject(std::thread::id expectedThreadId = {}, size_t maxEvents = 1000)
        : m_expectedThreadId(expectedThreadId)
        , m_maxEvents{ maxEvents }
    {
    }

    size_t eventsProcessed() const { return m_eventsProcessed; }

    void requestUpdate()
    {
        KDFoundation::EventLoop::instance()->postEvent(this, std::make_unique<KDFoundation::UpdateEvent>());
    }
    void requestCallback(const std::function<void()> &callback)
    {
        KDFoundation::EventLoop::instance()->postEvent(this, std::make_unique<CallbackEvent>(callback));
    }

    void quit()
    {
        m_stopRequested = true;
    }

protected:
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override
    {
        if (ev->type() == static_cast<KDFoundation::Event::Type>(static_cast<uint16_t>(KDFoundation::Event::Type::UserType) + 1)) {
            auto e = static_cast<CallbackEvent *>(ev);
            e->setAccepted(true);
            e->callback();
        }
        if (target == this && ev->type() == KDFoundation::Event::Type::Update) {
            if (m_expectedThreadId != std::thread::id{}) {
                CHECK(std::this_thread::get_id() == m_expectedThreadId);
            }
            ++m_eventsProcessed;
            ev->setAccepted(true);
            if (m_stopRequested || m_eventsProcessed == m_maxEvents) {
                KDFoundation::EventLoop::instance()->quit();
            } else {
                requestUpdate();
            }
        }

        KDFoundation::Object::event(target, ev);
    }

private:
    std::atomic_bool m_stopRequested = false;
    std::thread::id m_expectedThreadId;
    size_t m_maxEvents{ 0 };
    size_t m_eventsProcessed{ 0 };
};
