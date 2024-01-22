/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/position.h>
#include <array>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace KDGui;

TEST_SUITE("Position")
{
    TEST_CASE("Constructor")
    {
        const Position pos{ 1, 2 };
        REQUIRE_EQ(pos.x, 1);
        REQUIRE_EQ(pos.y, 2);
    }
    TEST_CASE("Equality")
    {
        const Position a{ 0, 2 };
        const Position b{ 3, 4 };
        const Position c{ 0, 0 };

        REQUIRE(a == a);
        REQUIRE(a != b);
        REQUIRE(a != c);
    }
    TEST_CASE("Math Operators")
    {
        const Position a{ 1, 2 };
        const Position b{ 3, 4 };

        REQUIRE_EQ(a + b, Position(4, 6));
        REQUIRE_EQ(a - b, Position(-2, -2));
        REQUIRE_EQ(a / int64_t{ 2 }, Position(0, 1));
    }
}
