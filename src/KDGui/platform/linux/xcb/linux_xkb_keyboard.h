/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/kdgui_keys.h>

#include <KDUtils/logging.h>

#include <xkbcommon/xkbcommon.h>
#include <xcb/xcb.h>

#include <memory>

namespace KDGui {

class LinuxXcbPlatformIntegration;

class LinuxXkbKeyboard
{
public:
    explicit LinuxXkbKeyboard(LinuxXcbPlatformIntegration *platformIntegration);
    ~LinuxXkbKeyboard();

    void handleKeyPress(xcb_key_press_event_t *ev);
    void handleKeyRelease(xcb_key_release_event_t *ev);

private:
    void registerForEvents();

    struct XkbContextDeleter {
        void operator()(xkb_context *context) const { return xkb_context_unref(context); }
    };
    using ScopedXkbContext = std::unique_ptr<xkb_context, XkbContextDeleter>;

    struct XkbKeymapDeleter {
        void operator()(xkb_keymap *keymap) const { return xkb_keymap_unref(keymap); }
    };
    using ScopedXkbKeymap = std::unique_ptr<xkb_keymap, XkbKeymapDeleter>;

    struct XkbStateDeleter {
        void operator()(xkb_state *state) const { return xkb_state_unref(state); }
    };
    using ScopedXkbState = std::unique_ptr<xkb_state, XkbStateDeleter>;

    std::shared_ptr<spdlog::logger> m_logger;
    LinuxXcbPlatformIntegration *m_platformIntegration{ nullptr };
    ScopedXkbContext m_context;
    ScopedXkbKeymap m_keymap;
    ScopedXkbState m_state;
    int32_t m_deviceId;
};

} // namespace KDGui
