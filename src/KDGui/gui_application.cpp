/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "gui_application.h"
#if defined(PLATFORM_LINUX)
#include <KDGui/platform/linux/xcb/linux_xcb_platform_integration.h>
#if defined(PLATFORM_WAYLAND)
#include <KDGui/platform/linux/wayland/linux_wayland_platform_integration.h>
#endif
#elif defined(PLATFORM_WIN32)
#include <KDGui/platform/win32/win32_gui_platform_integration.h>
#elif defined(PLATFORM_MACOS)
#include <KDGui/platform/cocoa/cocoa_platform_integration.h>
#elif defined(ANDROID)
#include <KDGui/platform/android/android_platform_integration.h>
#endif

#include <KDUtils/logging.h>

using namespace KDGui;

namespace {
#if defined(PLATFORM_LINUX)
std::unique_ptr<KDGui::AbstractGuiPlatformIntegration> createLinuxIntegration()
{
    bool prefersXcb = false;

    // TODO(doc): document this behavior
    if (const char *preferredPlatform = std::getenv("KDGUI_PLATFORM")) { // NOLINT(concurrency-mt-unsafe)
        prefersXcb = std::string_view{ preferredPlatform } == "xcb";
    }

#if defined(PLATFORM_WAYLAND)
    if (LinuxWaylandPlatformIntegration::checkAvailable() && !prefersXcb)
        return std::make_unique<LinuxWaylandPlatformIntegration>();
    else
#endif
        return std::make_unique<LinuxXcbPlatformIntegration>();
}
#endif

std::unique_ptr<AbstractGuiPlatformIntegration> createPlatformIntegration()
{
#if defined(ANDROID)
    return std::make_unique<AndroidPlatformIntegration>();
#elif defined(PLATFORM_LINUX)
    return createLinuxIntegration();
#elif defined(PLATFORM_WIN32)
    return std::make_unique<Win32GuiPlatformIntegration>();
#elif defined(PLATFORM_MACOS)
    return std::make_unique<CocoaPlatformIntegration>();
#endif
    return {};
}
} // namespace

GuiApplication::GuiApplication(std::unique_ptr<AbstractGuiPlatformIntegration> &&platformIntegration)
    : CoreApplication(platformIntegration ? std::move(platformIntegration) : createPlatformIntegration())
{
}
