/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/elapsedtimer.h>
#include <thread>

using namespace KDUtils;
using namespace std::literals::chrono_literals;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("ElaspedTimer")
{

    TEST_CASE("init")
    {
        static_assert(std::is_destructible<ElapsedTimer>{}, "ElapsedTimer should be destructible");
        static_assert(std::is_default_constructible<ElapsedTimer>{}, "ElapsedTimer should be default constructible");
        static_assert(std::is_copy_constructible<ElapsedTimer>{}, "ElapsedTimer should be copy constructible");
        static_assert(std::is_copy_assignable<ElapsedTimer>{}, "ElapsedTimer should copy assignable");
        static_assert(std::is_move_constructible<ElapsedTimer>{}, "ElapsedTimer should  move constructible");
        static_assert(std::is_move_assignable<ElapsedTimer>{}, "ElapsedTimer should move assignable");
    }

    TEST_CASE("checkAutoStarts")
    {
        // GIVEN
        ElapsedTimer t;

        // WHEN
        std::this_thread::sleep_for(500ms);
        ElapsedTimer::NsecDuration elapsed = t.elapsed();

        // THEN
        CHECK(elapsed >= 500ms);
        CHECK(elapsed <= 1000ms);
    }

    TEST_CASE("checkStart")
    {
        // GIVEN
        ElapsedTimer t;

        // WHEN
        std::this_thread::sleep_for(500ms);
        t.start();

        // AND
        std::this_thread::sleep_for(500ms);
        ElapsedTimer::NsecDuration elapsed = t.elapsed();

        // THEN
        CHECK(elapsed >= 500ms);
        CHECK(elapsed <= 1000ms);

        // WHEN
        std::this_thread::sleep_for(500ms);

        // THEN
        elapsed = t.elapsed();
        CHECK(elapsed >= 1000ms);
        CHECK(elapsed <= 1500ms);
    }

    TEST_CASE("checkRestart")
    {
        // GIVEN
        ElapsedTimer t;
        t.start();

        // WHEN
        std::this_thread::sleep_for(500ms);
        ElapsedTimer::NsecDuration elapsed = t.restart();

        // THEN
        CHECK(elapsed >= 500ms);
        CHECK(elapsed <= 1000ms);

        // WHEN
        std::this_thread::sleep_for(500ms);

        // THEN
        elapsed = t.elapsed();
        CHECK(elapsed >= 500ms);
        CHECK(elapsed <= 1000ms);
    }

    TEST_CASE("checkNsecElapsed")
    {
        // GIVEN
        ElapsedTimer t;
        t.start();

        // AND
        std::this_thread::sleep_for(500ms);
        const int64_t elapsed = t.nsecElapsed();

        // THEN
        CHECK(elapsed >= 500000000);
        CHECK(elapsed <= 1000000000);
    }

    TEST_CASE("checkMsecElapsed")
    {
        // GIVEN
        ElapsedTimer t;
        t.start();

        // AND
        std::this_thread::sleep_for(500ms);
        const int64_t elapsed = t.msecElapsed();

        // THEN
        CHECK(elapsed >= 500);
        CHECK(elapsed <= 1000);
    }
}
