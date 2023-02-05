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
#include <KDFoundation/logging.h>

#include <windows.h>

namespace KDGui {

class Win32GuiPlatformIntegration;

class Win32PlatformWindow : public AbstractPlatformWindow
{
public:
    explicit Win32PlatformWindow(Win32GuiPlatformIntegration *platformIntegration, Window *window);
    Win32PlatformWindow() = delete;

    virtual ~Win32PlatformWindow();

    Win32PlatformWindow(Win32PlatformWindow const &other) = delete;
    Win32PlatformWindow &operator=(Win32PlatformWindow const &other) = delete;

    Win32PlatformWindow(Win32PlatformWindow &&other) noexcept = default;
    Win32PlatformWindow &operator=(Win32PlatformWindow &&other) noexcept = default;

    HWND handle() const { return m_handle; }

    bool create() override;
    bool destroy() override;
    bool isCreated() override;

    void map() override;
    void unmap() override;

    Position queryCursorPosition() const;
    Position queryWindowSize() const;
    void disableCursor() override;
    void enableCursor() override;
    void updateCursor();

    void enableRawMouseInput() override;
    void disableRawMouseInput() override;
    bool isRawMouseInputEnabled() const { return m_rawMouseInputEnabled; }

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

    void handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key,
                        KeyboardModifiers modifiers) override;
    void handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key,
                          KeyboardModifiers modifiers) override;
    void handleTextInput(const std::string &str) override;

    void processRawInput(HRAWINPUT rawInput);

private:
    enum class CursorMode {
        Normal = 0,
        Disabled = 1
    };

    std::shared_ptr<spdlog::logger> m_logger;
    Win32GuiPlatformIntegration *m_platformIntegration;
    HWND m_handle{ nullptr };

    CursorMode m_cursorMode{ CursorMode::Normal };
    Position m_cursorRestorePosition{ 0, 0 };
    Position m_previousCursorPosition{ 0, 0 };
    Position m_previousWarpedCursorPosition{ 0, 0 };

    RAWINPUT *m_rawInputData{ nullptr };
    size_t m_rawInputDataSize{ 0 };
    bool m_rawMouseInputEnabled{ false };
};

} // namespace KDGui
