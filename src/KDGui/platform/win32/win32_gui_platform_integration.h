/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/win32/win32_platform_event_loop.h>
#include <KDGui/abstract_gui_platform_integration.h>
#include <KDUtils/logging.h>

#include "win32_platform_window.h"

#include <set>

namespace KDGui {

class Win32GuiPlatformIntegration : public AbstractGuiPlatformIntegration
{
public:
    Win32GuiPlatformIntegration();
    ~Win32GuiPlatformIntegration() override;

    std::shared_ptr<spdlog::logger> logger() { return m_logger; }

    bool registerWindowClass(const std::wstring &className, unsigned style, WNDPROC windowProc);

    AbstractClipboard *clipboard() override
    {
        // TODO(win32): Implement clipboard
        return nullptr;
    }

    std::string applicationDataPath(const KDFoundation::CoreApplication &app) const override;
    std::string assetsDataPath(const KDFoundation::CoreApplication &app) const override;

private:
    KDFoundation::Win32PlatformEventLoop *createPlatformEventLoopImpl() override;
    Win32PlatformWindow *createPlatformWindowImpl(Window *window) override;

    void unregisterWindowClasses();

    std::shared_ptr<spdlog::logger> m_logger;
    std::set<std::wstring> m_windowClasses;
};

} // namespace KDGui
