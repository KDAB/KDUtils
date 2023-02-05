/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/flags.h>

#include <numeric>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

enum Option {
    Option_Option1 = 0x0001,
    Option_Option2 = 0x0002,
    Option_Option3 = 0x0004,
    Option_Option4 = 0x0008,
    Option_Option5 = 0x0010,
    Option_Option6 = 0x0020,
    Option_Option7 = 0x0040,
    Option_Option8 = 0x0080,
};
using OptionFlags = Flags<Option>;

static_assert(std::is_destructible<OptionFlags>{});
static_assert(std::is_default_constructible<OptionFlags>{});
static_assert(std::is_copy_constructible<OptionFlags>{});
static_assert(std::is_copy_assignable<OptionFlags>{});
static_assert(std::is_move_constructible<OptionFlags>{});
static_assert(std::is_move_assignable<OptionFlags>{});

enum class AnotherOption : uint64_t {
    Option1 = 0x0001,
    Option2 = 0x0002,
    Option3 = 0x0004,
    Option4 = 0x0008,
    Option5 = 0x0010,
    Option6 = 0x0020,
    Option7 = 0x0040,
    Option8 = 0x0080,
};
using AnotherOptionFlags = Flags<AnotherOption>;

static_assert(std::is_destructible<AnotherOptionFlags>{});
static_assert(std::is_default_constructible<AnotherOptionFlags>{});
static_assert(std::is_copy_constructible<AnotherOptionFlags>{});
static_assert(std::is_copy_assignable<AnotherOptionFlags>{});
static_assert(std::is_move_constructible<AnotherOptionFlags>{});
static_assert(std::is_move_assignable<AnotherOptionFlags>{});
static_assert(std::is_same_v<AnotherOptionFlags::storage_type, uint64_t>);

TEST_CASE("Creation")
{
    SUBCASE("can default construct a Flags")
    {
        OptionFlags options;
        REQUIRE(options.none() == true);
        REQUIRE(options.any() == false);
    }

    SUBCASE("can default construct a Flags with enum class")
    {
        AnotherOptionFlags options;
        REQUIRE(options.none() == true);
        REQUIRE(options.any() == false);
    }

    SUBCASE("can value construct a Flags")
    {
        OptionFlags options(Option_Option1);
        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        REQUIRE(options.test(Option_Option1) == true);
        for (auto i = 1; i < 8; ++i) {
            const auto opt = static_cast<Option>(Option_Option1 << i);
            REQUIRE(options.test(opt) == false);
        }
    }

    SUBCASE("can value construct a Flags with enum class")
    {
        AnotherOptionFlags options(AnotherOption::Option1);
        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        REQUIRE(options.test(AnotherOption::Option1) == true);
        for (auto i = 1; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            REQUIRE(options.test(opt) == false);
        }
    }

    SUBCASE("can value construct a Flags with multiple flags set")
    {
        OptionFlags options(Option_Option1 | Option_Option2);
        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        REQUIRE(options.test(Option_Option1) == true);
        REQUIRE(options.test(Option_Option2) == true);
        for (auto i = 2; i < 8; ++i) {
            const auto opt = static_cast<Option>(Option_Option1 << i);
            REQUIRE(options.test(opt) == false);
        }
    }

    SUBCASE("can value construct a Flags with enum class with multiple flags set")
    {
        AnotherOptionFlags options(AnotherOption::Option1 | AnotherOption::Option2);
        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        REQUIRE(options.test(AnotherOption::Option1) == true);
        REQUIRE(options.test(AnotherOption::Option2) == true);
        for (auto i = 2; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            REQUIRE(options.test(opt) == false);
        }
    }
}

TEST_CASE("Casting")
{
    SUBCASE("can cast flags to underlying type")
    {
        OptionFlags options(Option_Option3);
        const auto optValue = options.operator std::underlying_type_t<Option>();
        REQUIRE(optValue == 4);
    }

    SUBCASE("can cast enum class flags to underlying type")
    {
        AnotherOptionFlags options(AnotherOption::Option4);
        const auto optValue = options.operator uint64_t();
        REQUIRE(optValue == 8);
    }

    SUBCASE("can cast flags to string")
    {
        OptionFlags options(Option_Option3);
        const auto optValue = options.operator std::string();
        REQUIRE(optValue == "00000000000000000000000000000100");
    }

    SUBCASE("can cast flags to string")
    {
        AnotherOptionFlags options(AnotherOption::Option4);
        const auto optValue = options.operator std::string();
        REQUIRE(optValue == "0000000000000000000000000000000000000000000000000000000000001000");
    }
}

TEST_CASE("Setting and testing")
{
    SUBCASE("can test if a given flag is set")
    {
        AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4);
        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = false;
            if (i == 1 || i == 3)
                expected = true;
            REQUIRE(options[opt] == expected);
            REQUIRE(options.test(opt) == expected);
        }
    }

    SUBCASE("can set a given flag")
    {
        AnotherOptionFlags options(AnotherOption::Option2);
        options.set(AnotherOption::Option6);

        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = false;
            if (i == 1 || i == 5)
                expected = true;
            REQUIRE(options[opt] == expected);
            REQUIRE(options.test(opt) == expected);
        }
    }

    SUBCASE("can clear all flags")
    {
        AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4);
        options.clear();
        REQUIRE(options.none() == true);
        REQUIRE(options.any() == false);
    }

    SUBCASE("can unset a flag")
    {
        AnotherOptionFlags options(AnotherOption::Option4);
        options.unset(AnotherOption::Option4);
        REQUIRE(options.none() == true);
        REQUIRE(options.any() == false);

        options.unset(AnotherOption::Option4);
        REQUIRE(options.none() == true);
        REQUIRE(options.any() == false);
    }

    SUBCASE("can invert a set of flags")
    {
        AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4);
        options.invert();

        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = true;
            if (i == 1 || i == 3)
                expected = false;
            REQUIRE(options[opt] == expected);
            REQUIRE(options.test(opt) == expected);
        }
    }

    SUBCASE("can retrieve the inverse of a set of flags")
    {
        const AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4);
        const auto options2 = options.inverse();

        REQUIRE(options2.none() == false);
        REQUIRE(options2.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = true;
            if (i == 1 || i == 3)
                expected = false;
            REQUIRE(options2[opt] == expected);
            REQUIRE(options2.test(opt) == expected);
        }
    }

    SUBCASE("can toggle a given flag")
    {
        AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4);
        options.toggle(AnotherOption::Option6);

        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = false;
            if (i == 1 || i == 3 || i == 5)
                expected = true;
            REQUIRE(options[opt] == expected);
            REQUIRE(options.test(opt) == expected);
        }

        options.toggle(AnotherOption::Option2);

        REQUIRE(options.none() == false);
        REQUIRE(options.any() == true);
        for (auto i = 0; i < 8; ++i) {
            const auto optValue = static_cast<AnotherOptionFlags::storage_type>(AnotherOption::Option1);
            const auto opt = static_cast<AnotherOption>(optValue << i);
            bool expected = false;
            if (i == 3 || i == 5)
                expected = true;
            REQUIRE(options[opt] == expected);
            REQUIRE(options.test(opt) == expected);
        }
    }

    SUBCASE("can get the count of set flags")
    {
        AnotherOptionFlags options(AnotherOption::Option2 | AnotherOption::Option4 | AnotherOption::Option6);
        REQUIRE(options.count() == 3);

        options.toggle(AnotherOption::Option4);
        REQUIRE(options.count() == 2);
    }
}
