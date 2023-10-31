/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_gui_platform_integration.h>
#include <KDGui/platform/linux/xcb/linux_xcb_platform_event_loop.h>
#include <KDGui/platform/linux/xcb/linux_xcb_platform_window.h>
#include <KDFoundation/logging.h>

#include <xcb/xcb.h>

#include <map>

namespace KDGui {

class LinuxXkbKeyboard;

class KDGUI_API LinuxXcbPlatformIntegration : public AbstractGuiPlatformIntegration
{
public:
    LinuxXcbPlatformIntegration();
    ~LinuxXcbPlatformIntegration() override;

    std::shared_ptr<spdlog::logger> logger() { return m_logger; }

    LinuxXkbKeyboard *keyboard();

    xcb_connection_t *connection() { return m_connection; }
    xcb_screen_t *screen() { return m_screen; }

    void registerWindowForEvents(xcb_window_t xcbWindow, LinuxXcbPlatformWindow *window)
    {
        m_windows.insert({ xcbWindow, window });
    }
    void unregisterWindowForEvents(xcb_window_t xcbWindow) { m_windows.erase(xcbWindow); }
    LinuxXcbPlatformWindow *window(xcb_window_t xcbWindow) { return m_windows.at(xcbWindow); }

    AbstractClipboard *clipboard() override
    {
        // TODO(xcb): Implement clipboard
        return nullptr;
    }

private:
    LinuxXcbPlatformEventLoop *createPlatformEventLoopImpl() override;
    LinuxXcbPlatformWindow *createPlatformWindowImpl(Window *window) override;

    bool initializeXkbExtension();
    void dumpScreenInfo(xcb_screen_t *screen);

    std::shared_ptr<spdlog::logger> m_logger;
    xcb_connection_t *m_connection{ nullptr };
    xcb_screen_t *m_screen{ nullptr };
    std::unique_ptr<LinuxXkbKeyboard> m_keyboard;
    std::map<xcb_window_t, LinuxXcbPlatformWindow *> m_windows;
};

} // namespace KDGui
