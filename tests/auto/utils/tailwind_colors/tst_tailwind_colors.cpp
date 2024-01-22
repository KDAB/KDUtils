/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/tailwind_colors.h>
#include <array>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Tailwind Color Utils")
{
    TEST_CASE("RGB color")
    {
        constexpr auto rgb = KDUtils::tailwindColorToRgb<std::array<float, 3>>(KDUtils::TailwindColor::Amber100);
        REQUIRE(rgb[0] == doctest::Approx(0.996078f));
        REQUIRE(rgb[1] == doctest::Approx(0.952941f));
        REQUIRE(rgb[2] == doctest::Approx(0.780392));
    }
    TEST_CASE("RGBA color")
    {
        constexpr auto rgb = KDUtils::tailwindColorToRgba<std::array<float, 4>>(KDUtils::TailwindColor::Blue100, 0.75f);
        REQUIRE(rgb[0] == doctest::Approx(0.858824f));
        REQUIRE(rgb[1] == doctest::Approx(0.917657f));
        REQUIRE(rgb[2] == doctest::Approx(0.996078f));
        REQUIRE(rgb[3] == doctest::Approx(0.75f));
    }
    TEST_CASE("Invalid color")
    {
        CHECK_THROWS_WITH_AS((KDUtils::tailwindColorToRgb<std::array<float, 3>>((KDUtils::TailwindColor)600)), "Invalid Tailwind color", std::runtime_error);
    }
}
