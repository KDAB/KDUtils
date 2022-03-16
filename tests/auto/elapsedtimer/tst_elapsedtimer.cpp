/*
    tst_elapsedtimer.cpp

    This file is part of Kuesa.

    Copyright (C) 2018-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Paul Lemire <paul.lemire@kdab.com>

    Licensees holding valid proprietary KDAB Kuesa licenses may use this file in
    accordance with the Kuesa Enterprise License Agreement provided with the Software in the
    LICENSE.KUESA.ENTERPRISE file.

    Contact info@kdab.com if any conditions of this licensing are not clear to you.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
