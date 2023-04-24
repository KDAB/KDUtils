/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/constexpr_sort.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

TEST_CASE("Simple type")
{
    SUBCASE("can create a sorted array of integers")
    {
        constexpr auto a = KDFoundation::sort(std::array{ 2, 5, 1, 8, 4 });

        constexpr std::array expectedResult{ 1, 2, 4, 5, 8 };
        for (auto i = 0; i < a.size(); ++i)
            REQUIRE(expectedResult[i] == a[i]);
    }
}

struct Entry {
    uint32_t x;
    uint32_t y;

    constexpr bool operator<(Entry const &other) const noexcept
    {
        return x < other.x;
    }

    bool operator==(Entry const &other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};

TEST_CASE("Struct")
{
    SUBCASE("can create a sorted array of structs")
    {
        constexpr auto a = KDFoundation::sort(
                std::array{
                        Entry{ 4, 11 },
                        Entry{ 1, 2 },
                        Entry{ 5, 14 },
                        Entry{ 3, 8 },
                        Entry{ 2, 5 } });

        constexpr std::array expectedResult{
            Entry{ 1, 2 },
            Entry{ 2, 5 },
            Entry{ 3, 8 },
            Entry{ 4, 11 },
            Entry{ 5, 14 }
        };

        for (auto i = 0; i < a.size(); ++i)
            REQUIRE(expectedResult[i] == a[i]);
    }
}
