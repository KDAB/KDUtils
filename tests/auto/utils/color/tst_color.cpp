/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/color.h>
#include <array>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("ColorUtils")
{
    TEST_CASE("Hexadecimal RGB Conversion")
    {
        constexpr auto rgb = KDUtils::hexToRgb<std::array<float, 3>>("#e2e8f0");
        REQUIRE(rgb[0] == doctest::Approx(226.0f / 255.0f));
        REQUIRE(rgb[1] == doctest::Approx(232.0f / 255.0f));
        REQUIRE(rgb[2] == doctest::Approx(240.0f / 255.0f));
    }
    TEST_CASE("Hexadecimal RGBA Conversion")
    {
        constexpr auto rgba = KDUtils::hexToRgba<std::array<float, 4>>("#e2e8f0", 0.75f);
        REQUIRE(rgba[0] == doctest::Approx(226.0f / 255.0f));
        REQUIRE(rgba[1] == doctest::Approx(232.0f / 255.0f));
        REQUIRE(rgba[2] == doctest::Approx(240.0f / 255.0f));
        REQUIRE(rgba[3] == doctest::Approx(0.75f));
    }
    TEST_CASE("Too long hexadecimal string")
    {
        CHECK_THROWS_WITH_AS((KDUtils::hexToRgb<std::array<float, 3>>("#e2e8f018711")), "Length of hexadecimal string must be 7 characters", std::runtime_error);
    }
    TEST_CASE("Hexadecimal missing a #")
    {
        CHECK_THROWS_WITH_AS((KDUtils::hexToRgb<std::array<float, 3>>("e2e8f0")), "Missing hashtag at start of hexadecimal string", std::runtime_error);
    }
}
