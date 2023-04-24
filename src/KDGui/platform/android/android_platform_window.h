/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: BogDan Vatra <bogdan@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_platform_window.h>
#include <KDGui/kdgui_global.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ANativeWindow;

#ifdef __cplusplus
}
#endif

namespace KDGui {

class AndroidPlatformIntegration;

class KDGUI_API AndroidPlatformWindow : public AbstractPlatformWindow
{
public:
    AndroidPlatformWindow(AndroidPlatformIntegration *androidPlatformIntegration, Window *window);
    bool create() final;
    bool destroy() final;
    bool isCreated() final;
    void map() final;
    void unmap() final;

    void disableCursor() final;
    void enableCursor() final;

    void enableRawMouseInput() final;
    void disableRawMouseInput() final;

    void setTitle(const std::string &title) final;
    void setSize(uint32_t width, uint32_t height) final;

    void handleResize(uint32_t width, uint32_t height) final;
    void handleMousePress(uint32_t timestamp, uint8_t button, int16_t xPos, int16_t yPos) final;
    void handleMouseRelease(uint32_t timestamp, uint8_t button, int16_t xPos, int16_t yPos) final;
    void handleMouseMove(uint32_t timestamp, uint8_t button, int64_t xPos, int64_t yPos) final;
    void handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta) final;
    void handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers) final;
    void handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers) final;
    void handleTextInput(const std::string &str) final;

    ANativeWindow *nativeWindow();

private:
    std::string m_title;
    AndroidPlatformIntegration *m_androidPlatformIntegrationSerenity;
};

} // namespace KDGui
