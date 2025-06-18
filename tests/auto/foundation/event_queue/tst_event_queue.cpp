/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/event.h>
#include <KDFoundation/event_queue.h>
#include <KDFoundation/object.h>

#include <numeric>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

static_assert(std::is_destructible<EventQueue>{});
static_assert(std::is_default_constructible<EventQueue>{});
static_assert(!std::is_copy_constructible<EventQueue>{});
static_assert(!std::is_copy_assignable<EventQueue>{});
static_assert(!std::is_move_constructible<EventQueue>{});
static_assert(!std::is_move_assignable<EventQueue>{});

namespace {
class MyEvent : public Event
{
public:
    MyEvent()
        : Event(static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 1))
    {
    }
};

class PayloadEvent : public Event
{
public:
    PayloadEvent(int x, int y)
        : Event(static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 2))
        , m_x{ x }
        , m_y{ y }
    {
    }

    int m_x;
    int m_y;
};

namespace {
void populateEventQueueWithNEvents(EventQueue &eventQueue, int n, std::vector<PayloadEvent *> &events, std::vector<std::unique_ptr<Object>> &targets)
{
    for (int i = 0; i < n; ++i) {
        auto ev = std::make_unique<PayloadEvent>(n + 1, 10 * (n + 1));
        events.push_back(ev.get());
        auto obj = std::make_unique<Object>();
        Object *target = obj.get();
        targets.push_back(std::move(obj));

        eventQueue.push(target, std::move(ev));
    }
}
} // namespace

TEST_CASE("Single-threaded use")
{
    SUBCASE("can push a posted event")
    {
        EventQueue eventQueue;
        auto ev = std::make_unique<PayloadEvent>(3, 4);
        PayloadEvent *originalEvent = ev.get();
        auto obj = std::make_unique<Object>();
        auto postEvent = std::make_unique<PostedEvent>(obj.get(), std::move(ev));
        eventQueue.push(std::move(postEvent));

        REQUIRE(eventQueue.size() == 1);
        PostedEvent *peekedEvent = eventQueue.peek();
        REQUIRE(peekedEvent->target() == obj.get());
        REQUIRE(peekedEvent->wrappedEvent() == originalEvent);
    }

    SUBCASE("can push an event to be wrapped in a posted event")
    {
        EventQueue eventQueue;
        auto ev = std::make_unique<PayloadEvent>(3, 4);
        PayloadEvent *originalEvent = ev.get();
        auto obj = std::make_unique<Object>();
        eventQueue.push(obj.get(), std::move(ev));

        REQUIRE(eventQueue.size() == 1);
        PostedEvent *peekedEvent = eventQueue.peek();
        REQUIRE(peekedEvent->target() == obj.get());
        REQUIRE(peekedEvent->wrappedEvent() == originalEvent);
    }

    SUBCASE("can push multiple events and then pop them in the correct order")
    {
        EventQueue eventQueue;
        std::vector<PayloadEvent *> events;
        std::vector<std::unique_ptr<Object>> targets;
        const int n = 10;

        populateEventQueueWithNEvents(eventQueue, n, events, targets);

        REQUIRE(eventQueue.size() == n);
        for (int i = 0; i < n; ++i) {
            auto postedEvent = eventQueue.tryPop();
            REQUIRE(postedEvent != std::unique_ptr<PostedEvent>());
            REQUIRE(eventQueue.size() == n - i - 1);
            REQUIRE(postedEvent->target() == targets[i].get());
            REQUIRE(postedEvent->wrappedEvent() == events[i]);
        }

        REQUIRE(eventQueue.size() == 0);
        REQUIRE(eventQueue.isEmpty());
    }

    SUBCASE("no event is removed when removeAllEventsTargeting() is called with EventReceiver that does not match any event")
    {
        EventQueue eventQueue;
        std::vector<PayloadEvent *> events;
        std::vector<std::unique_ptr<Object>> targets;
        const int n = 10;

        populateEventQueueWithNEvents(eventQueue, n, events, targets);

        auto obj = std::make_unique<Object>();
        Object *target = obj.get();
        eventQueue.removeAllEventsTargeting(*target);

        REQUIRE(eventQueue.size() == n);
    }

    SUBCASE("can push multiple events, remove one event and then pop them in the correct order")
    {
        EventQueue eventQueue;
        std::vector<PayloadEvent *> events;
        std::vector<std::unique_ptr<Object>> targets;
        int n = 10;

        populateEventQueueWithNEvents(eventQueue, n, events, targets);

        const auto idx = 5;
        eventQueue.removeAllEventsTargeting(*targets.at(idx));
        REQUIRE(eventQueue.size() == (n - 1));

        events.erase(events.begin() + idx);
        targets.erase(targets.begin() + idx);

        n -= 1;
        for (int i = 0; i < n; ++i) {
            auto postedEvent = eventQueue.tryPop();
            REQUIRE(postedEvent != std::unique_ptr<PostedEvent>());
            REQUIRE(eventQueue.size() == (n - i - 1));
            REQUIRE(postedEvent->target() == targets[i].get());
            REQUIRE(postedEvent->wrappedEvent() == events[i]);
        }

        REQUIRE(eventQueue.size() == 0);
        REQUIRE(eventQueue.isEmpty());
    }

    SUBCASE("can push multiple events, remove two events and then pop them in the correct order")
    {
        EventQueue eventQueue;
        std::vector<PayloadEvent *> events;
        std::vector<std::unique_ptr<Object>> targets;
        int n = 10;

        populateEventQueueWithNEvents(eventQueue, n, events, targets);

        eventQueue.removeAllEventsTargeting(*targets.front());
        eventQueue.removeAllEventsTargeting(*targets.back());
        REQUIRE(eventQueue.size() == (n - 2));

        events.erase(events.begin());
        targets.erase(targets.begin());
        events.erase(events.end() - 1);
        targets.erase(targets.end() - 1);

        n -= 2;
        for (int i = 0; i < n; ++i) {
            auto postedEvent = eventQueue.tryPop();
            REQUIRE(postedEvent != std::unique_ptr<PostedEvent>());
            REQUIRE(eventQueue.size() == (n - i - 1));
            REQUIRE(postedEvent->target() == targets[i].get());
            REQUIRE(postedEvent->wrappedEvent() == events[i]);
        }

        REQUIRE(eventQueue.size() == 0);
        REQUIRE(eventQueue.isEmpty());
    }
}
