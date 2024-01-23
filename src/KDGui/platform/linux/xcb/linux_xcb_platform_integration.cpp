/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xcb_platform_integration.h"
#include "linux_xkb_keyboard.h"

#include <KDUtils/logging.h>

#define explicit i_am_not_really_using_explicit
#include <xcb/xkb.h>
#undef explicit

using namespace KDGui;

LinuxXcbPlatformIntegration::LinuxXcbPlatformIntegration()
{
    int screenNumber;
    m_connection = xcb_connect(nullptr, &screenNumber);

    if (!initializeXkbExtension()) {
        SPDLOG_CRITICAL("Failed to initialize the xkb extension! Aborting.");
        throw std::runtime_error("Failed to initialize the xkb extension!");
    }

    const xcb_setup_t *setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

    for (int i = 0; i < screenNumber; ++i)
        xcb_screen_next(&iter);
    m_screen = iter.data;

    dumpScreenInfo(m_screen);

    m_clipboard = std::make_unique<LinuxXcbClipboard>(this);
}

LinuxXcbPlatformIntegration::~LinuxXcbPlatformIntegration()
{
    // Destroy Clipboard while connection is still valid
    m_clipboard.reset();

    xcb_disconnect(m_connection);
    if (m_logger)
        SPDLOG_LOGGER_DEBUG(m_logger, "Destroyed xcb_connection");
    else
        SPDLOG_DEBUG("Destroyed xcb_connection");
}

AbstractClipboard *LinuxXcbPlatformIntegration::clipboard()
{
    return m_clipboard.get();
}

LinuxXkbKeyboard *LinuxXcbPlatformIntegration::keyboard()
{
    if (!m_keyboard)
        m_keyboard = std::make_unique<LinuxXkbKeyboard>(this);
    return m_keyboard.get();
}

LinuxXcbPlatformWindow *LinuxXcbPlatformIntegration::window(xcb_window_t xcbWindow) const
{
    auto it = m_windows.find(xcbWindow);
    if (it == m_windows.end())
        return nullptr;
    return it->second;
}

LinuxXcbPlatformEventLoop *LinuxXcbPlatformIntegration::createPlatformEventLoopImpl()
{
    // We ensure that the logger has been created here rather than in the ctor so that
    // the central logging configuration in CoreApplication has had a chance to execute.
    m_logger = KDUtils::Logger::logger("xcb", spdlog::level::info);
    return new LinuxXcbPlatformEventLoop(this);
}

LinuxXcbPlatformWindow *LinuxXcbPlatformIntegration::createPlatformWindowImpl(Window *window)
{
    return new LinuxXcbPlatformWindow(this, window);
}

bool LinuxXcbPlatformIntegration::initializeXkbExtension()
{
    const auto reply = xcb_get_extension_data(m_connection, &xcb_xkb_id);
    if (!reply || !reply->present) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "XKeyboard extension is not provided by the X server");
        return false;
    }

    {
        const int requiredMajorVersion = 1;
        const int requiredMinorVersion = 0;
        xcb_generic_error_t *error = nullptr;
        auto cookie = xcb_xkb_use_extension(m_connection, requiredMajorVersion, requiredMinorVersion);
        auto reply = xcb_xkb_use_extension_reply(m_connection, cookie, &error);

        if (!reply) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to start the XKeyboard extension. Error code = {}", error->error_code);
            free(error);
            return false;
        }

        if (!reply->supported) {
            SPDLOG_LOGGER_CRITICAL(m_logger, "XKeyboard extension does not meet the minimum required version");
            free(reply);
            return false;
        }

        free(reply);
    }

    return true;
}

void LinuxXcbPlatformIntegration::dumpScreenInfo(xcb_screen_t *screen)
{
    SPDLOG_DEBUG("Information about screen {}", screen->root);
    SPDLOG_DEBUG("  width.........: {}", screen->width_in_pixels);
    SPDLOG_DEBUG("  height........: {}", screen->height_in_pixels);
    SPDLOG_DEBUG("  white pixel...: {}", screen->white_pixel);
    SPDLOG_DEBUG("  black pixel...: {}", screen->black_pixel);
}
