/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/logger.h>
using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Logger")
{

    TEST_CASE("can warn")
    {
        // GIVEN
        Logger l("test-category", Logger::Type::Warn);

        // THEN
        l.warn("test");
        l.warn("test {}", 28);
        int someVariable = 23 * 2;
        l.warn("test {}", someVariable);
    }

    TEST_CASE("can debug")
    {
        // GIVEN
        Logger l("test-category", Logger::Type::Debug);

        // THEN
        l.debug("test");
        l.debug("test {}", 28);
        int someVariable = 23 * 2;
        l.debug("test {}", someVariable);
    }

#define cWarning(category, ...) KDUtils::Logger(#category, KDUtils::Logger::Type::Warn).warn(__VA_ARGS__)
    TEST_CASE("temporary logger logs")
    {
        std::string str = "string - " + std::to_string(23);
        cWarning(test - category, "test str: {}", str);
        cWarning(test - category, "test");
    }
}
