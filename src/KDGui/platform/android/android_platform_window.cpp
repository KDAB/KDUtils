/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: BogDan Vatra <bogdan@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_platform_window.h"

#include <KDFoundation/core_application.h>
#include <KDGui/gui_events.h>
#include <KDGui/window.h>

#include <android_native_app_glue.h>

#include "android_platform_integration.h"

using namespace KDFoundation;
namespace KDGui {

AndroidPlatformWindow::AndroidPlatformWindow(AndroidPlatformIntegration *androidPlatformIntegrationSerenity, Window *window)
    : AbstractPlatformWindow(window, Type::Android)
    , m_androidPlatformIntegrationSerenity(androidPlatformIntegrationSerenity)
{
}

bool AndroidPlatformWindow::create()
{
    if (AndroidPlatformIntegration::s_androidApp->userData) {
        m_androidPlatformIntegrationSerenity->registerPlatformWindow(this);
        const auto width = ANativeWindow_getWidth(AndroidPlatformIntegration::s_androidApp->window);
        const auto height = ANativeWindow_getHeight(AndroidPlatformIntegration::s_androidApp->window);
        if (m_window->width.get() != width || m_window->height.get() != height) {
            m_androidPlatformIntegrationSerenity->handleWindowResize();
        }
        return true;
    }
    return false;
}

bool AndroidPlatformWindow::destroy()
{
    m_androidPlatformIntegrationSerenity->unregisterPlatformWindow(this);
    return true;
}

bool AndroidPlatformWindow::isCreated()
{
    return AndroidPlatformIntegration::s_androidApp->userData;
}

void AndroidPlatformWindow::map() { }

void AndroidPlatformWindow::unmap() { }

void AndroidPlatformWindow::disableCursor() { }

void AndroidPlatformWindow::enableCursor() { }

void AndroidPlatformWindow::enableRawMouseInput() { }

void AndroidPlatformWindow::disableRawMouseInput() { }

void AndroidPlatformWindow::setTitle(const std::string &title)
{
    m_title = title;
}

void AndroidPlatformWindow::setSize(uint32_t /*width*/, uint32_t /*height*/) { }

void AndroidPlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    ResizeEvent ev{ width, height };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleMousePress(uint32_t timestamp, KDGui::MouseButtons buttons, int16_t xPos, int16_t yPos)
{
    MousePressEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleMouseRelease(uint32_t timestamp, KDGui::MouseButtons buttons, int16_t xPos, int16_t yPos)
{
    MouseReleaseEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleMouseMove(uint32_t timestamp, KDGui::MouseButtons buttons, int64_t xPos, int64_t yPos)
{
    MouseMoveEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
{
    MouseWheelEvent ev{ timestamp, xDelta, yDelta };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyPressEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyReleaseEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void AndroidPlatformWindow::handleTextInput(const std::string &str)
{
    TextInputEvent ev{ str };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

ANativeWindow *AndroidPlatformWindow::nativeWindow()
{
    return AndroidPlatformIntegration::s_androidApp->window;
}

} // namespace KDGui
