/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: BogDan Vatra <bogdan@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_platform_integration.h"

#include <android_native_app_glue.h>
#include <KDUtils/file.h>
#include <KDFoundation/platform/android/android_platform_event_loop.h>
#include <KDFoundation/platform/android/android_platform_integration.h>
#include <KDFoundation/core_application.h>

#include <KDGui/window.h>

#include <dlfcn.h>
#include <filesystem>

#include "android_platform_window.h"
#include "android_keyboard_map.h"

#include <android/log.h>

namespace KDGui {

KDFoundation::AbstractPlatformEventLoop *AndroidPlatformIntegration::createPlatformEventLoopImpl()
{
    auto eventLoop = new KDFoundation::AndroidPlatformEventLoop(this);

    // Connect to system event signals
    m_resizeEventConnection = eventLoop->windowResize.connect(&AndroidPlatformIntegration::handleWindowResize, this);
    m_keyEventConnection = eventLoop->keyEvent.connect(&AndroidPlatformIntegration::handleKeyEvent, this);
    m_touchEventConnection = eventLoop->touchEvent.connect(&AndroidPlatformIntegration::handleTouchEvent, this);

    return eventLoop;
}

AbstractPlatformWindow *AndroidPlatformIntegration::createPlatformWindowImpl(Window *window)
{
    return new AndroidPlatformWindow(this, window);
}

AndroidPlatformIntegration::AndroidPlatformIntegration()
{
    KDUtils::setAssetManager(KDFoundation::AndroidPlatformIntegration::s_androidApp->activity->assetManager);
}

void AndroidPlatformIntegration::handleWindowResize()
{
    const int width = ANativeWindow_getWidth(KDFoundation::AndroidPlatformIntegration::s_androidApp->window);
    const int height = ANativeWindow_getHeight(KDFoundation::AndroidPlatformIntegration::s_androidApp->window);
    for (auto *platformWindow : m_windows) {
        const auto *w = platformWindow->window();
        if (w->width.get() != width || w->height.get() != height) {
            platformWindow->handleResize(width, height);
        }
    }
}
void AndroidPlatformIntegration::handleKeyEvent(int32_t action, int32_t code, int32_t meta, int64_t time)
{
    auto key = androidKeyCodeToKey(code);
    auto modifiers = androidKeyMetaToModifiers(meta);

    if (action == AKEY_EVENT_ACTION_DOWN) {
        for (auto *platformWindow : m_windows) {
            platformWindow->handleKeyPress(static_cast<uint32_t>(time), static_cast<uint8_t>(code), key, modifiers);
        }

    } else if (action == AKEY_EVENT_ACTION_UP) {
        for (auto *platformWindow : m_windows) {
            platformWindow->handleKeyRelease(static_cast<uint32_t>(time), static_cast<uint8_t>(code), key, modifiers);
        }
    }
}

void AndroidPlatformIntegration::handleTouchEvent(int32_t action, int64_t xPos, int64_t yPos, int64_t time)
{
    const auto mouseButton = LeftButton;

    for (auto *platformWindow : m_windows) {

        switch (action) {
        case AMOTION_EVENT_ACTION_DOWN:
            platformWindow->handleMousePress(time, mouseButton, xPos, yPos);
            break;
        case AMOTION_EVENT_ACTION_UP:
            platformWindow->handleMouseRelease(time, mouseButton, xPos, yPos);
            break;
        case AMOTION_EVENT_ACTION_MOVE:
            platformWindow->handleMouseMove(time, mouseButton, xPos, yPos);
            break;
        default:
            __android_log_print(ANDROID_LOG_INFO, "KDGuiAndroid",
                                "touch event not handled: %d", action);
            break;
        }
    }
}

void AndroidPlatformIntegration::registerPlatformWindow(AbstractPlatformWindow *window)
{
    m_windows.insert(window);
}

void AndroidPlatformIntegration::unregisterPlatformWindow(AbstractPlatformWindow *window)
{
    m_windows.erase(window);
}

} // namespace KDGui
