/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#include <KDGui/platform/linux/xcb/linux_xcb_platform_integration.h>
#include <KDGui/window.h>

#include <KDUtils/logging.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDGui;

static_assert(std::is_destructible<LinuxXcbPlatformWindow>{});
static_assert(!std::is_default_constructible<LinuxXcbPlatformWindow>{});
static_assert(!std::is_copy_constructible<LinuxXcbPlatformWindow>{});
static_assert(!std::is_copy_assignable<LinuxXcbPlatformWindow>{});
static_assert(std::is_nothrow_move_constructible<LinuxXcbPlatformWindow>{});
static_assert(std::is_nothrow_move_assignable<LinuxXcbPlatformWindow>{});

TEST_CASE("Creation")
{
    spdlog::set_level(spdlog::level::debug);

    SUBCASE("can create a linux platform window")
    {
        auto w = std::make_unique<Window>();
        auto platformIntegration = std::make_unique<LinuxXcbPlatformIntegration>();
        auto platformWindow = std::make_unique<LinuxXcbPlatformWindow>(platformIntegration.get(), w.get());
        REQUIRE(platformWindow->window() == w.get());
        REQUIRE(platformWindow->type() == AbstractPlatformWindow::Type::XCB);
    }
}
