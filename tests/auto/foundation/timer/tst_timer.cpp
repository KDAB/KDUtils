/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/timer.h>
#include <KDFoundation/core_application.h>

#include <numeric>
#include <string>
#include <thread>
#include <chrono>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

TEST_CASE("Timer Basic Tests")
{
    CoreApplication app;

    SUBCASE("Timer Initial State")
    {
        Timer timer;
        CHECK_FALSE(timer.running());
        CHECK_EQ(timer.interval(), std::chrono::microseconds(0));
        CHECK_FALSE(timer.singleShot());
    }

    SUBCASE("Timer Properties")
    {
        Timer timer;

        // Test setting and getting the interval
        const auto testInterval = std::chrono::microseconds(500000); // 500ms
        timer.interval = testInterval;
        CHECK_EQ(timer.interval(), testInterval);

        // Test setting and getting running state
        timer.running = true;
        CHECK(timer.running());
        timer.running = false;
        CHECK_FALSE(timer.running());

        // Test setting and getting single shot mode
        timer.singleShot = true;
        CHECK(timer.singleShot());
        timer.singleShot = false;
        CHECK_FALSE(timer.singleShot());
    }
}

TEST_CASE("Timer Timeout Signal Tests")
{
    CoreApplication app;

    SUBCASE("Timer Emits Timeout Signal")
    {
        Timer timer;
        bool timeoutEmitted = false;

        // Connect to timeout signal
        auto connection = timer.timeout.connect([&timeoutEmitted]() {
            timeoutEmitted = true;
        });

        // Set interval to 50ms for quick test
        timer.interval = std::chrono::milliseconds(50);

        // Start timer
        timer.running = true;

        // Process events to allow timer to fire
        // Wait a bit longer than the timer interval
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Check that timeout was emitted
        CHECK(timeoutEmitted);

        // Stop timer
        timer.running = false;
    }

    SUBCASE("Single Shot Timer Stops After Timeout")
    {
        Timer timer;
        int timeoutCount = 0;

        // Connect to timeout signal
        auto connection = timer.timeout.connect([&timeoutCount]() {
            timeoutCount++;
        });

        // Configure as single shot timer with 50ms interval
        timer.singleShot = true;
        timer.interval = std::chrono::milliseconds(50);

        // Start timer
        timer.running = true;

        // Wait for first timeout
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Check that timeout was emitted once and timer stopped
        CHECK_EQ(timeoutCount, 1);
        CHECK_FALSE(timer.running()); // Timer should have stopped itself

        // Wait longer to verify no more timeouts occur
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Count should still be 1
        CHECK_EQ(timeoutCount, 1);
    }

    SUBCASE("Regular Timer Continues After Timeout")
    {
        Timer timer;
        int timeoutCount = 0;

        // Connect to timeout signal
        auto connection = timer.timeout.connect([&timeoutCount]() {
            timeoutCount++;
        });

        // Configure as regular (non-single-shot) timer with 50ms interval
        timer.singleShot = false;
        timer.interval = std::chrono::milliseconds(50);

        // Start timer
        timer.running = true;

        // Wait long enough for multiple timeouts
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        app.processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        app.processEvents();

        // Check that multiple timeouts were emitted and timer still running
        CHECK_GT(timeoutCount, 1);
        CHECK(timer.running.get()); // Timer should still be running

        // Stop timer
        timer.running = false;
    }
}
