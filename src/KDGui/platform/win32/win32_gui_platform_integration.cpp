/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "win32_gui_platform_integration.h"

#include <KDFoundation/platform/win32/win32_platform_integration.h>
#include "win32_platform_window.h"
#include "win32_utils.h"

#include <KDUtils/logging.h>

using namespace KDFoundation;
using namespace KDGui;

Win32GuiPlatformIntegration::Win32GuiPlatformIntegration()
    : m_logger{ KDUtils::Logger::logger("win32", spdlog::level::info) }
{
}

Win32GuiPlatformIntegration::~Win32GuiPlatformIntegration()
{
    unregisterWindowClasses();
}

Win32PlatformEventLoop *Win32GuiPlatformIntegration::createPlatformEventLoopImpl()
{
    return new Win32PlatformEventLoop();
}

Win32PlatformWindow *Win32GuiPlatformIntegration::createPlatformWindowImpl(Window *window)
{
    return new Win32PlatformWindow(this, window);
}

bool Win32GuiPlatformIntegration::registerWindowClass(const std::wstring &className, unsigned style, WNDPROC windowProc)
{
    const auto alreadyRegistered = m_windowClasses.find(className) != m_windowClasses.end();
    if (alreadyRegistered)
        return true;

    const auto appInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = style;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = appInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = className.c_str();
    wc.hIcon = static_cast<HICON>(LoadImage(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

    const auto atom = RegisterClassEx(&wc);
    if (!atom) {
        SPDLOG_CRITICAL("Failed to register window class: {}", windowsErrorMessage(GetLastError()));
    }

    m_windowClasses.insert(className);

    return atom != 0;
}

void Win32GuiPlatformIntegration::unregisterWindowClasses()
{
    const auto appInstance = GetModuleHandle(nullptr);

    for (const auto &name : m_windowClasses) {
        if (!UnregisterClass(name.c_str(), appInstance)) {
            SPDLOG_CRITICAL("Failed to unregister class: {}", windowsErrorMessage(GetLastError()));
        }
    }

    m_windowClasses.clear();
}
