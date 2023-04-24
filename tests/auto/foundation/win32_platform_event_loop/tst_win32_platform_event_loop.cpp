/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <KDFoundation/platform/win32/win32_platform_event_loop.h>
#include <KDFoundation/object.h>

#include <KDFoundation/logging.h>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>

using namespace KDFoundation;

static_assert(std::is_destructible<Win32PlatformEventLoop>{});
static_assert(std::is_default_constructible<Win32PlatformEventLoop>{});
static_assert(!std::is_copy_constructible<Win32PlatformEventLoop>{});
static_assert(!std::is_copy_assignable<Win32PlatformEventLoop>{});
static_assert(!std::is_move_constructible<Win32PlatformEventLoop>{});
static_assert(!std::is_move_assignable<Win32PlatformEventLoop>{});

TEST_CASE("Wait for events")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can poll for events (0ms timeout)")
    {
        Win32PlatformEventLoop loop;
        loop.waitForEvents(0);
    }

    SUBCASE("can wait for events (100 ms timeout)")
    {
        Win32PlatformEventLoop loop;
        loop.waitForEvents(100);
    }

    SUBCASE("can wake up by calling wakeUp from another thread")
    {
        Win32PlatformEventLoop loop;

        // Spawn a thread to wake up the event loop
        std::mutex mutex;
        std::condition_variable cond;
        bool ready = false;
        auto callWakeUp = [&mutex, &cond, &ready, &loop]() {
            spdlog::info("Launched helper thread");
            std::unique_lock lock(mutex);
            cond.wait(lock, [&ready] { return ready == true; });
            spdlog::info("Thread going to sleep before waking up event loop");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            loop.wakeUp();
        };
        std::thread t1(callWakeUp);

        // Kick the thread off
        {
            spdlog::info("Waking up helper thread");
            std::unique_lock lock(mutex);
            ready = true;
            cond.notify_all();
        }

        const auto startTime = std::chrono::steady_clock::now();
        loop.waitForEvents(10000);
        const auto endTime = std::chrono::steady_clock::now();

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        spdlog::info("elapsedTime = {}", elapsedTime);
        REQUIRE(elapsedTime < 10000);

        // Be nice!
        t1.join();
    }
}
