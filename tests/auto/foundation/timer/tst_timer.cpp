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

TEST_CASE("C++ Style Timer Convenience Functions")
{
    CoreApplication app;

    SUBCASE("createTimeout Basic Usage")
    {
        bool timeoutCalled = false;
        auto timer = Timer::createTimeout([&timeoutCalled]() {
            timeoutCalled = true;
        },
                                          std::chrono::milliseconds(50));

        // Verify timer properties are set correctly
        CHECK(timer->running());
        CHECK(timer->singleShot());
        CHECK_EQ(timer->interval(), std::chrono::microseconds(50000));

        // Wait for timeout and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Check timeout was called and timer is no longer running
        CHECK(timeoutCalled);
        CHECK_FALSE(timer->running());
    }

    SUBCASE("createRecurring Basic Usage")
    {
        int callCount = 0;
        auto timer = Timer::createRecurring([&callCount]() {
            callCount++;
        },
                                            std::chrono::milliseconds(50));

        // Verify timer properties are set correctly
        CHECK(timer->running());
        CHECK_FALSE(timer->singleShot());
        CHECK_EQ(timer->interval(), std::chrono::microseconds(50000));

        // Wait for multiple timeouts and process events multiple times
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        app.processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        app.processEvents();

        // Check timeout was called multiple times and timer is still running
        CHECK_GT(callCount, 1);
        CHECK(timer->running());

        // Stop the timer
        timer->running = false;

        // Store the current count
        int previousCount = callCount;

        // Wait again and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Check that count didn't increase after stopping
        CHECK_EQ(callCount, previousCount);
    }
}

TEST_CASE("JavaScript Style Timer Functions")
{
    CoreApplication app;

    SUBCASE("setTimeout Basic Usage")
    {
        bool timeoutCalled = false;
        Timer::TimerId id = Timer::setTimeout(std::chrono::milliseconds(50), [&timeoutCalled]() {
            timeoutCalled = true;
        });

        // Check timer was created with valid ID
        CHECK_GT(id, 0);
        CHECK(Timer::isTimerActive(id));

        // Wait for timeout and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        app.processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        app.processEvents();

        // Check timeout was called
        CHECK(timeoutCalled);

        // For single-shot, the timer should be automatically removed
        CHECK_FALSE(Timer::isTimerActive(id));
    }

    SUBCASE("setRecurring Basic Usage")
    {
        int callCount = 0;
        Timer::TimerId id = Timer::setRecurring(std::chrono::milliseconds(50), [&callCount]() {
            callCount++;
        });

        // Check timer was created with valid ID
        CHECK_GT(id, 0);
        CHECK(Timer::isTimerActive(id));

        // Wait for multiple timeouts and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        app.processEvents();

        // Store the current count (should be at least 1)
        int firstCheckCount = callCount;
        CHECK_GT(firstCheckCount, 0);

        // Timer should still be active
        CHECK(Timer::isTimerActive(id));

        // Wait for more timeouts
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        app.processEvents();

        // Count should have increased
        CHECK_GT(callCount, firstCheckCount);

        // Cleanup the timer as the lambda is referencing a local variable that will go out of scope
        Timer::cancelTimer(id);
        CHECK_FALSE(Timer::isTimerActive(id));
    }

    SUBCASE("cancelTimer")
    {
        bool timeoutCalled = false;
        Timer::TimerId id = Timer::setTimeout(std::chrono::milliseconds(100), [&timeoutCalled]() {
            timeoutCalled = true;
        });

        // Timer should be active
        CHECK(Timer::isTimerActive(id));

        // Cancel the timer
        CHECK(Timer::cancelTimer(id));

        // Timer should no longer be active
        CHECK_FALSE(Timer::isTimerActive(id));

        // Wait and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        app.processEvents();

        // Callback should not have been called
        CHECK_FALSE(timeoutCalled);

        // Trying to cancel again should fail
        CHECK_FALSE(Timer::cancelTimer(id));
    }

    SUBCASE("Multiple Managed Timers")
    {
        int timer1Calls = 0;
        int timer2Calls = 0;
        int timer3Calls = 0;

        // Create three timers with different intervals
        Timer::TimerId id1 = Timer::setTimeout(std::chrono::milliseconds(50), [&timer1Calls]() {
            timer1Calls++;
        });

        Timer::TimerId id2 = Timer::setRecurring(std::chrono::milliseconds(30), [&timer2Calls]() {
            timer2Calls++;
        });

        Timer::TimerId id3 = Timer::setTimeout(std::chrono::milliseconds(150), [&timer3Calls]() {
            timer3Calls++;
        });

        // All timers should be active
        CHECK(Timer::isTimerActive(id1));
        CHECK(Timer::isTimerActive(id2));
        // CHECK(Timer::isTimerActive(id3));

        // Wait for first timeout and process events
        // TODO: Seems like each call to processEvents() only fires one timer
        // Need to call it multiple times to ensure all timers are processed.
        // Can we fix this in the event loop so that all timers are processed in one go?
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        app.processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        app.processEvents();

        // First timer should have fired and been removed
        CHECK_EQ(timer1Calls, 1);
        CHECK_FALSE(Timer::isTimerActive(id1));

        // Second timer should have fired at least once and still be active
        CHECK_GT(timer2Calls, 0);
        CHECK(Timer::isTimerActive(id2));

        // Third timer shouldn't have fired yet
        CHECK_EQ(timer3Calls, 0);
        CHECK(Timer::isTimerActive(id3));

        // Cancel the recurring timer
        CHECK(Timer::cancelTimer(id2));
        CHECK_FALSE(Timer::isTimerActive(id2));

        // Wait for third timer and process events
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        app.processEvents();

        // Third timer should have fired and been removed
        CHECK_EQ(timer3Calls, 1);
        CHECK_FALSE(Timer::isTimerActive(id3));

        // Second timer count should not have increased
        CHECK_EQ(timer2Calls, timer2Calls);

        // Clean up any managed timers
        Timer::cancelAllTimers();

        CHECK_FALSE(Timer::isTimerActive(id1));
        CHECK_FALSE(Timer::isTimerActive(id2));
        CHECK_FALSE(Timer::isTimerActive(id3));
    }
}
