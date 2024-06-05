/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/config.h>
#include <KDFoundation/core_application.h>
#include <KDFoundation/timer.h>
#include <KDFoundation/event.h>
#include <KDUtils/logging.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

static_assert(std::is_destructible<CoreApplication>{});
static_assert(std::is_default_constructible<CoreApplication>{});
static_assert(!std::is_copy_constructible<CoreApplication>{});
static_assert(!std::is_copy_assignable<CoreApplication>{});
static_assert(!std::is_move_constructible<CoreApplication>{});
static_assert(!std::is_move_assignable<CoreApplication>{});

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

class EventObject : public Object
{
public:
    EventObject(int expectedX, int expectedY)
        : Object()
        , m_expectedX{ expectedX }
        , m_expectedY{ expectedY }
    {
    }

    bool userEventDelivered() const { return m_userEventDelivered; }

protected:
    void userEvent(Event *ev) override
    {
        if (ev->type() == static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 2)) {
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

class RecursiveEventPosterObject : public Object
{
public:
    RecursiveEventPosterObject()
        : Object()
    {
    }

    size_t eventsProcessed() const { return m_eventsProcessed; }

    void requestUpdate()
    {
        CoreApplication::instance()->postEvent(this, std::make_unique<UpdateEvent>());
    }

protected:
    void event(EventReceiver *target, Event *ev) override
    {
        if (target == this && ev->type() == Event::Type::Update) {
            ++m_eventsProcessed;
            ev->setAccepted(true);
            requestUpdate();
        }

        Object::event(target, ev);
    }

private:
    size_t m_eventsProcessed{ 0 };
};

TEST_CASE("Creation")
{
    SUBCASE("default construction")
    {
        CoreApplication app;
        REQUIRE(CoreApplication::instance() != nullptr);
    }

    SUBCASE("Can access one and only instance via instance function")
    {
        REQUIRE(CoreApplication::instance() == nullptr);
        auto *app = new CoreApplication;
        REQUIRE(CoreApplication::instance() == app);
        delete app;
        REQUIRE(CoreApplication::instance() == nullptr);
    }
}

TEST_CASE("Event handling")
{
    SUBCASE("can post an event")
    {
        CoreApplication app;
        auto ev = std::make_unique<PayloadEvent>(5, 6);
        auto obj = app.createChild<Object>();
        app.postEvent(obj, std::move(ev));
        REQUIRE(app.eventQueueSize() == 1);
    }

    SUBCASE("calling processEvents processes an event in the queue")
    {
        CoreApplication app;
        auto ev = std::make_unique<PayloadEvent>(5, 6);
        auto obj = app.createChild<EventObject>(5, 6);
        app.postEvent(obj, std::move(ev));
        REQUIRE(app.eventQueueSize() == 1);

        app.processEvents();
        REQUIRE(obj->userEventDelivered() == true);
        REQUIRE(app.eventQueueSize() == 0);
    }

    SUBCASE("don't send pending events for deleted objects")
    {
        CoreApplication app;
        auto ev = std::make_unique<PayloadEvent>(5, 6);
        auto obj = new EventObject(5, 6);
        app.postEvent(obj, std::move(ev));

        delete obj;

        app.processEvents();
    }

    SUBCASE("don't end up in infinite loop")
    {
        // GIVEN
        CoreApplication app;
        auto obj = std::make_unique<RecursiveEventPosterObject>();

        // WHEN
        obj->requestUpdate();
        app.processEvents();

        // THEN
        CHECK(obj->eventsProcessed() == 1);

        // WHEN
        app.processEvents();

        // THEN
        CHECK(obj->eventsProcessed() == 2);

        // WHEN
        for (size_t i = 0; i < 10; ++i)
            app.processEvents();

        // THEN
        CHECK(obj->eventsProcessed() == 12);
    }
}

TEST_CASE("Timer handling")
{
    SUBCASE("timer fires correctly")
    {
        using namespace std::literals::chrono_literals;

        CoreApplication app;
        Timer timer;
        timer.interval = 100ms;
        timer.running = true;

        int timeout = 0;
        auto startTime = std::chrono::steady_clock::now();
        auto time = startTime;
        timer.timeout.connect([&]() {
            const auto endTime = std::chrono::steady_clock::now();
            REQUIRE(endTime - time > 50ms);
            REQUIRE(endTime - time < 150ms);
            time = endTime;
            timeout++;
        });

        while (std::chrono::steady_clock::now() - startTime < 500ms) {
            app.processEvents(500);
        }
        REQUIRE(timeout > 3);
        REQUIRE(timeout < 8);

        startTime = std::chrono::steady_clock::now();
        int oldTimeout = timeout;
        timer.running = false;

        while (std::chrono::steady_clock::now() - startTime < 500ms) {
            app.processEvents(500);
        }
        REQUIRE(timeout == oldTimeout);
    }

    SUBCASE("timer restarts after timeout change")
    {
        using namespace std::chrono_literals;

        CoreApplication app;
        Timer timer;

        // Set initial interval to 100ms
        timer.interval = 100ms;
        timer.running = true;

        bool fired = false;

        timer.timeout.connect([&] {
            fired = true;
        });

        // After 50ms, timer shouldn't have yet fired
        app.processEvents(50);
        REQUIRE(fired == false);

        // Reset interval, it should fire 150ms from now
        timer.interval = 150ms;

        // Advance 100ms, it should _not_ yet fire because it was
        // restarted to 150ms even though the original timeout passed
        app.processEvents(100);
        REQUIRE(fired == false);

        // It should fire after additional 100ms
        app.processEvents(100);
        REQUIRE(fired == true);
    }
}

TEST_CASE("Main event loop")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can enter the main event loop and quit from another thread")
    {
        CoreApplication app;

        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto quitTheApp = [&mutex, &cond, &ready, &app]() {
            SPDLOG_INFO("Launched worker thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready == true; });
            SPDLOG_INFO("Worker thread going to sleep before quitting the app event loop");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            SPDLOG_INFO("Worker thread requesting the app to quit");
            app.quit();
        };
        std::thread t1(quitTheApp);

        {
            SPDLOG_INFO("Waking up helper thread");
            std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        const auto startTime = std::chrono::steady_clock::now();
        SPDLOG_INFO("Main thread entering event loop");
        app.exec();
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        SPDLOG_INFO("elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 1000);

        t1.join();
    }
}
