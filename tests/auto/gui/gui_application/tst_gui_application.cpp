/*
 *  This file is part of KDUtils.
 *
 *  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 *  Author: Paul Lemire <paul.lemire@kdab.com>
 *  Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Contact KDAB at <info@kdab.com> for commercial licensing options.
 */

#include "../../foundation/common/event_mockups.h"

#include <KDFoundation/event_loop.h>
#include <KDFoundation/timer.h>
#include <KDGui/gui_application.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;
using namespace KDGui;

TEST_CASE("Creation")
{
    SUBCASE("default construction")
    {
        const GuiApplication app;
        REQUIRE(GuiApplication::instance() != nullptr);
    }

    SUBCASE("Can access one and only instance via instance function")
    {
        REQUIRE(GuiApplication::instance() == nullptr);
        auto *app = new GuiApplication;
        REQUIRE(GuiApplication::instance() == app);
        delete app;
        REQUIRE(GuiApplication::instance() == nullptr);
    }
}

TEST_CASE("Timer handling" * doctest::timeout(120))
{
    SUBCASE("Timer fires on correct thread")
    {
        using namespace std::literals::chrono_literals;

        CoreApplication app;

        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        int timeoutCount = 0;

        auto runWorkerThread = [&mutex, &cond, &ready, &app, &timeoutCount]() {
            SPDLOG_INFO("Launched worker thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready; });

            EventLoop loop;

            auto workerThreadId = std::this_thread::get_id();
            Timer timer;
            timer.interval = 100ms;
            timer.running = true;

            std::ignore = timer.timeout.connect([&]() {
                CHECK(std::this_thread::get_id() == workerThreadId);
                timeoutCount++;
                timer.running = false;
                loop.quit();
                app.quit();
            });

            loop.exec();
        };
        std::thread t1(runWorkerThread);

        {
            SPDLOG_INFO("Waking up worker thread");
            const std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        // std::this_thread::sleep_for(5000ms);
        app.exec();
        REQUIRE(timeoutCount == 1);

        t1.join();
    }
}

TEST_CASE("Main event loop")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can enter the main event loop and quit from another thread")
    {
        GuiApplication app;

        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto quitTheApp = [&mutex, &cond, &ready, &app]() {
            SPDLOG_INFO("Launched worker thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready; });
            SPDLOG_INFO("Worker thread going to sleep before quitting the app event loop");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            SPDLOG_INFO("Worker thread requesting the app to quit");
            app.quit();
        };
        std::thread t1(quitTheApp);

        {
            SPDLOG_INFO("Waking up helper thread");
            const std::unique_lock lock(mutex);
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

TEST_CASE("Worker thread event loop")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("Can create and execute an event loop in a worker thread")
    {
        GuiApplication app;

        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;

        auto runWorkerThread = [&mutex, &cond, &ready, &app]() {
            SPDLOG_INFO("Launched worker thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready; });

            EventLoop loop;

            auto ev = std::make_unique<PayloadEvent>(5, 6);
            auto obj = app.createChild<EventObject>(5, 6);
            loop.postEvent(obj, std::move(ev));
            REQUIRE(loop.eventQueueSize() == 1);
            REQUIRE(app.eventQueueSize() == 0);

            loop.processEvents();
            REQUIRE(obj->userEventDelivered() == true);
            REQUIRE(app.eventQueueSize() == 0);

            loop.quit();
            const auto startTime = std::chrono::steady_clock::now();
            SPDLOG_INFO("Worker thread entering event loop");
            loop.exec();
            const auto endTime = std::chrono::steady_clock::now();

            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            SPDLOG_INFO("elapsedTime = {}", elapsedTime);
            REQUIRE(elapsedTime < 1000);
        };
        std::thread t1(runWorkerThread);

        {
            SPDLOG_INFO("Waking up worker thread");
            const std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        t1.join();
    }

    SUBCASE("Can run main loop and worker thread event loop simultaneously")
    {
        GuiApplication app;

        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;

        auto runWorkerThread = [&mutex, &cond, &ready, &app]() {
            SPDLOG_INFO("Launched worker thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready; });

            EventLoop loop;

            auto obj = std::make_unique<RecursiveEventPosterObject>(std::this_thread::get_id(), 16);
            obj->requestUpdate();

            // Check that event is posted on current thread's event loop
            loop.processEvents();
            CHECK(obj->eventsProcessed() == 1);

            const auto startTime = std::chrono::steady_clock::now();
            SPDLOG_INFO("Worker thread entering event loop");
            loop.exec();
            const auto endTime = std::chrono::steady_clock::now();

            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            SPDLOG_INFO("Worker thread: elapsedTime = {}", elapsedTime);
            REQUIRE(elapsedTime < 1000);

            CHECK(obj->eventsProcessed() == 16);

            SPDLOG_INFO("Worker thread requesting the app to quit");
            app.quit();
        };
        std::thread t1(runWorkerThread);

        {
            SPDLOG_INFO("Waking up helper thread");
            const std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        auto obj = std::make_unique<RecursiveEventPosterObject>(std::this_thread::get_id(), 5);
        obj->requestUpdate();

        const auto startTime = std::chrono::steady_clock::now();
        SPDLOG_INFO("Main thread entering event loop");
        app.exec();
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        SPDLOG_INFO("Main thread: elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 1000);

        CHECK(obj->eventsProcessed() == 5);

        t1.join();
    }

    SUBCASE("Worker thread's event loop can outlive GuiApplication")
    {
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;

        std::thread worker_thread;

        {
            const GuiApplication app;

            auto runWorkerThread = [&mutex, &cond, &ready]() {
                SPDLOG_INFO("Launched worker thread");

                EventLoop loop;

                auto obj = std::make_unique<RecursiveEventPosterObject>(std::this_thread::get_id(), 128);
                obj->requestUpdate();

                {
                    SPDLOG_INFO("Waking up main thread");
                    const std::unique_lock lock(mutex);
                    ready = true;
                    cond.notify_all();
                }

                const auto startTime = std::chrono::steady_clock::now();
                SPDLOG_INFO("Worker thread entering event loop");
                loop.exec();
                const auto endTime = std::chrono::steady_clock::now();

                auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                SPDLOG_INFO("Worker thread: elapsedTime = {}", elapsedTime);
                REQUIRE(elapsedTime < 1000);

                CHECK(obj->eventsProcessed() == 128);
            };
            worker_thread = std::thread(runWorkerThread);

            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready; });
        }

        worker_thread.join();
    }
}
