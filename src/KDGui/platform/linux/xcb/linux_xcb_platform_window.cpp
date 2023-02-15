/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xcb_platform_window.h"
#include <KDGui/gui_events.h>
#include "linux_xcb_platform_integration.h"
#include <KDGui/window.h>

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>

#include <xcb/xcb.h>

#include <array>

using namespace KDFoundation;
using namespace KDGui;

LinuxXcbPlatformWindow::LinuxXcbPlatformWindow(
        LinuxXcbPlatformIntegration *platformIntegration,
        Window *window)
    : AbstractPlatformWindow(window)
    , m_platformIntegration{ platformIntegration }
{
}

bool LinuxXcbPlatformWindow::create()
{
    if (m_xcbWindow)
        return false;

    const auto connection = m_platformIntegration->connection();
    const auto screen = m_platformIntegration->screen();

    m_xcbWindow = xcb_generate_id(connection);
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[1] = {
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY
    };
    xcb_create_window(
            connection, /* Connection          */
            XCB_COPY_FROM_PARENT, /* depth (same as root)*/
            m_xcbWindow, /* window Id           */
            screen->root, /* parent window       */
            0, 0, /* x, y                */
            m_window->width.get(),
            m_window->height.get(),
            10, /* border_width        */
            XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class               */
            screen->root_visual, /* visual              */
            mask,
            values);

    const auto title = m_window->title.get();
    xcb_change_property(
            connection,
            XCB_PROP_MODE_REPLACE,
            m_xcbWindow,
            XCB_ATOM_WM_NAME,
            XCB_ATOM_STRING,
            8,
            title.length(),
            title.data());

    // Register for the WM_DELETE_WINDOW client message events. This prevents the
    // server from disconnecting us when a top-level window is closed. Allows us to
    // just unmap the window gracefully.
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t *reply2 = xcb_intern_atom_reply(connection, cookie2, nullptr);
    m_closeAtom = reply2->atom;
    xcb_change_property(
            connection,
            XCB_PROP_MODE_REPLACE,
            m_xcbWindow,
            (*reply).atom,
            XCB_ATOM_ATOM,
            32,
            1,
            &(*reply2).atom);

    // Register our window id with the integration so that it can map xcb events back to us
    // when they occur.
    m_platformIntegration->registerWindowForEvents(m_xcbWindow, this);

    // Create a hidden cursor from an unset 1x1 pixmap
    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    m_hiddenCursor = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, pixmap, screen->root, 1, 1);
    xcb_create_cursor(connection, m_hiddenCursor, pixmap, pixmap,
                      0, 0, 0, 0,
                      0, 0, 0, 0);
    xcb_free_pixmap(connection, pixmap);

    if (window()->cursorEnabled.get() == false)
        disableCursor();

    xcb_flush(connection);

    return true;
}

bool LinuxXcbPlatformWindow::destroy()
{
    const auto connection = m_platformIntegration->connection();

    // Destroy the hidden cursor
    xcb_free_cursor(connection, m_hiddenCursor);
    m_hiddenCursor = 0;

    // Destroy the window and unregister for events
    xcb_destroy_window(connection, m_xcbWindow);
    xcb_flush(connection);

    m_platformIntegration->unregisterWindowForEvents(m_xcbWindow);
    m_xcbWindow = 0;
    return true;
}

void LinuxXcbPlatformWindow::map()
{
    const auto connection = m_platformIntegration->connection();
    xcb_map_window(connection, m_xcbWindow);
    xcb_flush(connection);
}

void LinuxXcbPlatformWindow::unmap()
{
    const auto connection = m_platformIntegration->connection();
    xcb_unmap_window(connection, m_xcbWindow);
    xcb_flush(connection);
}

Position LinuxXcbPlatformWindow::queryCursorPosition() const
{
    const auto connection = m_platformIntegration->connection();
    auto cookie = xcb_query_pointer(connection, m_xcbWindow);
    auto reply = xcb_query_pointer_reply(connection, cookie, nullptr);
    Position pos{ reply->win_x, reply->win_y };
    return pos;
}

Position LinuxXcbPlatformWindow::queryWindowSize() const
{
    const auto connection = m_platformIntegration->connection();
    auto cookie = xcb_get_geometry(connection, m_xcbWindow);
    auto reply = xcb_get_geometry_reply(connection, cookie, nullptr);
    Position size{ reply->width, reply->height };
    return size;
}

void LinuxXcbPlatformWindow::disableCursor()
{
    // Remember where the cursor is
    m_cursorRestorePosition = queryCursorPosition();

    // Hide the cursor
    const auto connection = m_platformIntegration->connection();
    uint32_t values[] = { m_hiddenCursor };
    xcb_change_window_attributes(connection, m_xcbWindow, XCB_CURSOR, &values);

    // Move cursor to centre of the window
    const auto windowSize = queryWindowSize();
    xcb_warp_pointer(connection, XCB_NONE, m_xcbWindow,
                     0, 0, 0, 0,
                     int16_t(windowSize.x / 2), int16_t(windowSize.y / 2));
    m_previousWarpedCursorPosition = windowSize / 2;

    // Grab the cursor
    xcb_grab_pointer(connection,
                     1, // Still report events to the window
                     m_xcbWindow,
                     XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
                     XCB_GRAB_MODE_ASYNC,
                     XCB_GRAB_MODE_ASYNC,
                     m_xcbWindow,
                     m_hiddenCursor,
                     XCB_CURRENT_TIME);
    xcb_flush(connection);

    m_cursorMode = CursorMode::Disabled;
}

void LinuxXcbPlatformWindow::enableCursor()
{
    m_cursorMode = CursorMode::Normal;

    // Ungrab the cursor
    const auto connection = m_platformIntegration->connection();
    xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);

    // Restore the cursor position
    xcb_warp_pointer(connection, XCB_NONE, m_xcbWindow,
                     0, 0, 0, 0,
                     int16_t(m_cursorRestorePosition.x), int16_t(m_cursorRestorePosition.y));

    // Reset the default cursor (inherit from parent window)
    uint32_t values[] = { 0 };
    xcb_change_window_attributes(connection, m_xcbWindow, XCB_CURSOR, &values);
    xcb_flush(connection);
}

void LinuxXcbPlatformWindow::enableRawMouseInput()
{
    // TODO: Implement me!
}

void LinuxXcbPlatformWindow::disableRawMouseInput()
{
    // TODO: Implement me!
}

void LinuxXcbPlatformWindow::setTitle(const std::string &title)
{
    if (!m_xcbWindow)
        return;

    const auto connection = m_platformIntegration->connection();
    xcb_change_property(
            connection,
            XCB_PROP_MODE_REPLACE,
            m_xcbWindow,
            XCB_ATOM_WM_NAME,
            XCB_ATOM_STRING,
            8,
            title.length(),
            title.data());
    xcb_flush(connection);
}

void LinuxXcbPlatformWindow::setSize(uint32_t width, uint32_t height)
{
    if (!m_xcbWindow)
        return;

    const auto connection = m_platformIntegration->connection();
    std::array<uint32_t, 2> size = { width, height };
    xcb_configure_window(
            connection,
            m_xcbWindow,
            XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
            size.data());
    xcb_flush(connection);
}

void LinuxXcbPlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    // Pass a resize event to the window
    KDFoundation::ResizeEvent ev{ width, height };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleMousePress(uint32_t timestamp, uint8_t button,
                                              int16_t xPos, int16_t yPos)
{
    MousePressEvent ev{ timestamp, button, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleMouseRelease(uint32_t timestamp, uint8_t button,
                                                int16_t xPos, int16_t yPos)
{
    MouseReleaseEvent ev{ timestamp, button, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleMouseMove(uint32_t timestamp, uint8_t button,
                                             int64_t xPos, int64_t yPos)
{
    Position pos{ xPos, yPos };
    const bool processMouseMove = pos != m_previousWarpedCursorPosition;

    if (processMouseMove && m_cursorMode == CursorMode::Disabled) {
        const Position delta = pos - m_previousCursorPosition;
        pos = window()->cursorPosition.get() + delta;
    }

    m_previousCursorPosition = Position(xPos, yPos);

    // Re-center the cursor if in Disabled cursor mode
    if (m_cursorMode == CursorMode::Disabled) {
        const auto windowSize = queryWindowSize();
        const Position center{ static_cast<int64_t>(windowSize.x / 2), static_cast<int64_t>(windowSize.y / 2) };
        if (m_previousCursorPosition != center) {
            const auto connection = m_platformIntegration->connection();
            xcb_warp_pointer(connection, XCB_NONE, m_xcbWindow,
                             0, 0, 0, 0,
                             int16_t(center.x), int16_t(center.y));
            m_previousWarpedCursorPosition = center;
        }
    }

    if (processMouseMove) {
        MouseMoveEvent ev{ timestamp, button, pos.x, pos.y };
        CoreApplication::instance()->sendEvent(m_window, &ev);
    }
}

void LinuxXcbPlatformWindow::handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
{
    MouseWheelEvent ev{ timestamp, xDelta, yDelta };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleKeyPress(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
{
    KeyPressEvent ev{ timestamp, nativeKeycode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleKeyRelease(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
{
    KeyReleaseEvent ev{ timestamp, nativeKeycode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxXcbPlatformWindow::handleTextInput(const std::string &str)
{
    TextInputEvent ev{ str };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

xcb_connection_t *LinuxXcbPlatformWindow::connection() const
{
    return m_platformIntegration->connection();
}
