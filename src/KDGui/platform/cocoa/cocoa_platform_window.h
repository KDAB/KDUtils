/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_platform_window.h>

#include <objc/objc-runtime.h>
namespace KDGui {

class CocoaPlatformWindow : public AbstractPlatformWindow
{
public:
    explicit CocoaPlatformWindow(Window *window);
    CocoaPlatformWindow() = delete;

    virtual ~CocoaPlatformWindow();

    CocoaPlatformWindow(CocoaPlatformWindow const &other) = delete;
    CocoaPlatformWindow &operator=(CocoaPlatformWindow const &other) = delete;

    CocoaPlatformWindow(CocoaPlatformWindow &&other) noexcept = default;
    CocoaPlatformWindow &operator=(CocoaPlatformWindow &&other) noexcept = default;

    id nativeWindow() const { return m_nativeWindow; }

    bool create() override;
    bool destroy() override;
    bool isCreated() override;

    void map() override;
    void unmap() override;

    void disableCursor() override;
    void enableCursor() override;

    void enableRawMouseInput() override;
    void disableRawMouseInput() override;

    void grabMouse() override;
    void releaseMouse() override;

    void setTitle(const std::string &title) override;

    void setSize(uint32_t width, uint32_t height) override;

    void handleResize(uint32_t width, uint32_t height) override;
    void handleMousePress(uint32_t timestamp, MouseButton button, int16_t xPos, int16_t yPos) override;
    void handleMouseRelease(uint32_t timestamp, MouseButton button, int16_t xPos, int16_t yPos) override;
    void handleMouseMove(uint32_t timestamp, MouseButton button, int64_t xPos, int64_t yPos) override;
    void handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta) override;
    void handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers) override;
    void handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers) override;
    void handleTextInput(const std::string &str) override;

private:
    id m_nativeWindow = nil;
    id m_delegate = nil;
    id m_view = nil;
};

} // namespace KDGui
