/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_platform_window.h>
#include <KDGui/position.h>
#include <KDGui/kdgui_global.h>

#include <xcb/xproto.h>

namespace KDGui {

class LinuxXcbPlatformIntegration;

class KDGUI_API LinuxXcbPlatformWindow : public AbstractPlatformWindow
{
public:
    explicit LinuxXcbPlatformWindow(LinuxXcbPlatformIntegration *platformIntegraton,
                                    Window *window);

    LinuxXcbPlatformWindow() = delete;

    LinuxXcbPlatformWindow(LinuxXcbPlatformWindow const &other) = delete;
    LinuxXcbPlatformWindow &operator=(LinuxXcbPlatformWindow const &other) = delete;

    LinuxXcbPlatformWindow(LinuxXcbPlatformWindow &&other) noexcept = default;
    LinuxXcbPlatformWindow &operator=(LinuxXcbPlatformWindow &&other) noexcept = default;

    bool create() override;
    bool destroy() override;
    bool isCreated() override { return m_xcbWindow != 0; }

    xcb_atom_t closeAtom() { return m_closeAtom; }
    void map() override;
    void unmap() override;

    Position queryCursorPosition() const;
    Position queryWindowSize() const;
    void disableCursor() override;
    void enableCursor() override;

    void enableRawMouseInput() override;
    void disableRawMouseInput() override;

    void setTitle(const std::string &title) override;

    void setSize(uint32_t width, uint32_t height) override;
    void handleResize(uint32_t width, uint32_t height) override;

    void handleMousePress(
            uint32_t timestamp, uint8_t button,
            int16_t xPos, int16_t yPos) override;

    void handleMouseRelease(
            uint32_t timestamp, uint8_t button,
            int16_t xPos, int16_t yPos) override;

    void handleMouseMove(
            uint32_t timestamp, uint8_t button,
            int64_t xPos, int64_t yPos) override;

    void handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta) override;

    void handleKeyPress(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers) override;
    void handleKeyRelease(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers) override;
    void handleTextInput(const std::string &str) override;

private:
    enum class CursorMode {
        Normal = 0,
        Disabled = 1
    };

    LinuxXcbPlatformIntegration *m_platformIntegration;
    xcb_window_t m_xcbWindow{ 0 };
    xcb_atom_t m_closeAtom{ 0 };
    xcb_cursor_t m_hiddenCursor{ 0 };
    CursorMode m_cursorMode{ CursorMode::Normal };
    Position m_cursorRestorePosition{ 0, 0 };
    Position m_previousCursorPosition{ 0, 0 };
    Position m_previousWarpedCursorPosition{ 0, 0 };
};

} // namespace KDGui
