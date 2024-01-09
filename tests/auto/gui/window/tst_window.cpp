/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/window.h>
#include <KDFoundation/core_application.h>
#include <KDGui/gui_application.h>
#include <KDGui/config.h>

#include <KDUtils/logging.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;
using namespace KDGui;

static_assert(std::is_destructible<Window>{});
static_assert(std::is_default_constructible<Window>{});
static_assert(!std::is_copy_constructible<Window>{});
static_assert(!std::is_copy_assignable<Window>{});
static_assert(std::is_nothrow_move_constructible<Window>{});
static_assert(std::is_nothrow_move_assignable<Window>{});

TEST_CASE("Creation")
{
    spdlog::set_level(spdlog::level::debug);

    {
        // Just used to load the logging configuration from file once up front.
        // This prevents e.g. Window from creating its own logger which then causes
        // CoreApplication ctor to fail in later tests.
        CoreApplication app;
    }

    SUBCASE("can create a window")
    {
        auto w = std::make_unique<Window>();
        REQUIRE(w->title.get() == "KDGui");
        REQUIRE(w->visible.get() == false);
        REQUIRE(w->platformWindow() == nullptr);
    }

    SUBCASE("creating a platform window without an app fails")
    {
        auto w = std::make_unique<Window>();
        w->create();
        REQUIRE(w->platformWindow() == nullptr);
    }

    SUBCASE("creating a platform window with a core app fails")
    {
        CoreApplication app;
        auto w = std::make_unique<Window>();
        w->create();
        REQUIRE(w->platformWindow() == nullptr);
    }

    SUBCASE("creating a platform window with a gui app succeeds")
    {
        GuiApplication app;
        auto w = std::make_unique<Window>();
        w->create();
        REQUIRE(w->platformWindow() != nullptr);
#if defined(KDGUI_PLATFORM_WIN32)
        REQUIRE(w->platformWindow()->type() == AbstractPlatformWindow::Type::Win32);
#elif defined(KDGUI_PLATFORM_XCB) || defined(KDGUI_PLATFORM_WAYLAND)
        const bool isXcb = w->platformWindow()->type() == AbstractPlatformWindow::Type::XCB;
        const bool isWayland = w->platformWindow()->type() == AbstractPlatformWindow::Type::Wayland;
        const bool isXcbOrWayland = isXcb || isWayland;
        REQUIRE(isXcbOrWayland);
#elif defined(KDGUI_PLATFORM_COCOA)
        REQUIRE(w->platformWindow()->type() == AbstractPlatformWindow::Type::Cocoa);
#endif
    }
}
