/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/event.h>

#include <numeric>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

static_assert(std::is_destructible<Event>{});
static_assert(!std::is_default_constructible<Event>{});
static_assert(!std::is_copy_constructible<Event>{});
static_assert(!std::is_copy_assignable<Event>{});
static_assert(std::is_nothrow_move_constructible<Event>{});
static_assert(std::is_nothrow_move_assignable<Event>{});

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

TEST_CASE("Construction")
{
    SUBCASE("can create an Event")
    {
        auto ev = std::make_unique<Event>(Event::Type::KeyPress);

        REQUIRE(ev->type() == Event::Type::KeyPress);
        REQUIRE_FALSE(ev->isSystemEvent());
        REQUIRE(ev->isManualEvent());
        REQUIRE_FALSE(ev->isAccepted());
    }

    SUBCASE("can create an Event subclass instance")
    {
        auto ev = std::make_unique<MyEvent>();

        REQUIRE(ev->type() == static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 1));
        REQUIRE_FALSE(ev->isSystemEvent());
        REQUIRE(ev->isManualEvent());
        REQUIRE_FALSE(ev->isAccepted());
    }

    SUBCASE("can create an Event subclass instance with arguments")
    {
        auto ev = std::make_unique<PayloadEvent>(4, 9);

        REQUIRE(ev->type() == static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 2));
        REQUIRE_FALSE(ev->isSystemEvent());
        REQUIRE(ev->isManualEvent());
        REQUIRE_FALSE(ev->isAccepted());
        REQUIRE(ev->m_x == 4);
        REQUIRE(ev->m_y == 9);
    }
}

TEST_CASE("Accepting events")
{
    SUBCASE("can mark an event as accepted")
    {
        auto ev = std::make_unique<Event>(Event::Type::KeyPress);
        ev->setAccepted(true);

        REQUIRE(ev->type() == Event::Type::KeyPress);
        REQUIRE_FALSE(ev->isSystemEvent());
        REQUIRE(ev->isManualEvent());
        REQUIRE(ev->isAccepted());
    }

    SUBCASE("can mark an event as not accepted")
    {
        auto ev = std::make_unique<Event>(Event::Type::KeyPress);
        ev->setAccepted(false);

        REQUIRE(ev->type() == Event::Type::KeyPress);
        REQUIRE_FALSE(ev->isSystemEvent());
        REQUIRE(ev->isManualEvent());
        REQUIRE_FALSE(ev->isAccepted());
    }
}
