/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_wayland_platform_window.h"
#include <KDGui/gui_events.h>
#include <KDGui/window.h>
#include "linux_wayland_platform_integration.h"
#include "linux_wayland_platform_output.h"

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>

#include <array>
#include <assert.h>

using namespace KDFoundation;
using namespace KDGui;

LinuxWaylandPlatformWindow::LinuxWaylandPlatformWindow(
        LinuxWaylandPlatformIntegration *platformIntegration,
        Window *window)
    : AbstractPlatformWindow(window)
    , m_platformIntegration{ platformIntegration }
{
    CoreApplication::instance()->applicationName.valueChanged().connect([this]() {
        if (m_toplevel) {
            setAppId();
        }
    });
    window->scaleFactor.valueChanged().connect([this](float value) {
        if (m_surface) {
            wl_surface_set_buffer_scale(m_surface, value);
        }
    });
}

LinuxWaylandPlatformWindow *LinuxWaylandPlatformWindow::fromSurface(wl_surface *surface)
{
    auto w = static_cast<LinuxWaylandPlatformWindow *>(wl_surface_get_user_data(surface));
    assert(w);
    return w;
}

bool LinuxWaylandPlatformWindow::create()
{
    if (m_surface)
        return true;

    m_surface = wl_compositor_create_surface(m_platformIntegration->compositor().object);
    static const wl_surface_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformWindow::enter>,
        wrapWlCallback<&LinuxWaylandPlatformWindow::leave>
    };
    wl_surface_add_listener(m_surface, &listener, this);

    wl_surface_set_buffer_scale(m_surface, window()->scaleFactor());

    return true;
}

bool LinuxWaylandPlatformWindow::destroy()
{
    if (m_surface) {
        wl_surface_destroy(m_surface);
    }
    return true;
}

void LinuxWaylandPlatformWindow::map()
{
    if (m_xdgSurface)
        return;

    m_xdgSurface = xdg_wm_base_get_xdg_surface(m_platformIntegration->xdgShell().object, m_surface);
    static const xdg_surface_listener surfListener = {
        wrapWlCallback<&LinuxWaylandPlatformWindow::configure>
    };
    xdg_surface_add_listener(m_xdgSurface, &surfListener, this);

    m_toplevel = xdg_surface_get_toplevel(m_xdgSurface);
    static const xdg_toplevel_listener toplevelListener = {
        wrapWlCallback<&LinuxWaylandPlatformWindow::configureToplevel>,
        wrapWlCallback<&LinuxWaylandPlatformWindow::close>,
#ifdef XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION
        wrapWlCallback<&LinuxWaylandPlatformWindow::configureBounds>,
#endif
    };
    xdg_toplevel_add_listener(m_toplevel, &toplevelListener, this);
    setAppId();

    wl_surface_commit(m_surface);
    wl_display_roundtrip_queue(m_platformIntegration->display(), m_platformIntegration->queue());
}

void LinuxWaylandPlatformWindow::unmap()
{
    xdg_toplevel_destroy(m_toplevel);
    xdg_surface_destroy(m_xdgSurface);
    m_xdgSurface = nullptr;
}

void LinuxWaylandPlatformWindow::disableCursor()
{
    m_cursorMode = CursorMode::Disabled;

    cursorChanged.emit();
}

void LinuxWaylandPlatformWindow::enableCursor()
{
    m_cursorMode = CursorMode::Normal;
    cursorChanged.emit();
}

void LinuxWaylandPlatformWindow::enableRawMouseInput()
{
}

void LinuxWaylandPlatformWindow::disableRawMouseInput()
{
}

void LinuxWaylandPlatformWindow::setTitle(const std::string &title)
{
    xdg_toplevel_set_title(m_toplevel, title.c_str());
}

void LinuxWaylandPlatformWindow::setSize(uint32_t width, uint32_t height)
{
    if (m_xdgSurface) {
        xdg_surface_set_window_geometry(m_xdgSurface, 0, 0, width, height);
    }
}

void LinuxWaylandPlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    // Pass a resize event to the window
    ResizeEvent ev{ width, height };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMousePress(uint32_t timestamp, uint8_t button,
                                                  int16_t xPos, int16_t yPos)
{
    MousePressEvent ev{ timestamp, button, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMouseRelease(uint32_t timestamp, uint8_t button,
                                                    int16_t xPos, int16_t yPos)
{
    MouseReleaseEvent ev{ timestamp, button, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMouseMove(uint32_t timestamp, uint8_t button,
                                                 int64_t x, int64_t y)
{
    if (m_cursorMode == CursorMode::Normal) {
        MouseMoveEvent ev{ timestamp, button, x, y };
        CoreApplication::instance()->sendEvent(m_window, &ev);
    }
}

void LinuxWaylandPlatformWindow::handleMouseMoveRelative(uint32_t timestamp, int64_t dx, int64_t dy)
{
    if (m_cursorMode == CursorMode::Disabled) {
        Position pos = window()->cursorPosition.get() + Position(dx, dy);
        MouseMoveEvent ev{ timestamp, 0, pos.x, pos.y };
        CoreApplication::instance()->sendEvent(m_window, &ev);
    }
}

void LinuxWaylandPlatformWindow::handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
{
    MouseWheelEvent ev{ timestamp, xDelta, yDelta };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleKeyPress(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
{
    KeyPressEvent ev{ timestamp, nativeKeycode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleKeyRelease(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
{
    KeyReleaseEvent ev{ timestamp, nativeKeycode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleTextInput(const std::string &str)
{
    TextInputEvent ev{ str };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::enter(wl_surface *surface, wl_output *output)
{
    auto o = LinuxWaylandPlatformOutput::fromOutput(output);
    m_enteredOutputs.push_back(o);

    updateScaleFactor();
}

void LinuxWaylandPlatformWindow::leave(wl_surface *surface, wl_output *output)
{
    auto o = LinuxWaylandPlatformOutput::fromOutput(output);
    auto it = std::find(m_enteredOutputs.begin(), m_enteredOutputs.end(), o);
    if (it != m_enteredOutputs.end()) {
        m_enteredOutputs.erase(it);
    }

    updateScaleFactor();
}

void LinuxWaylandPlatformWindow::configure(xdg_surface *xdgSurface, uint32_t serial)
{
    xdg_surface_ack_configure(xdgSurface, serial);
}

void LinuxWaylandPlatformWindow::configureToplevel(xdg_toplevel *toplevel, int32_t width, int32_t height, wl_array *states)
{
    if (width != 0 && height != 0) {
        handleResize(width, height);
    }
}

void LinuxWaylandPlatformWindow::close(xdg_toplevel *toplevel)
{
    SPDLOG_LOGGER_DEBUG(m_platformIntegration->logger(), "Window closed by user");
    window()->visible = false;
}

void LinuxWaylandPlatformWindow::configureBounds(xdg_toplevel *toplevel, int32_t width, int32_t height)
{
    int32_t w = window()->width();
    int32_t h = window()->height();
    if (width != 0 && width < w) {
        w = width;
    }
    if (height != 0 && height < h) {
        h = height;
    }

    if (w != window()->width() || h != window()->height()) {
        handleResize(w, h);
    }
}

void LinuxWaylandPlatformWindow::setAppId()
{
    xdg_toplevel_set_app_id(m_toplevel, CoreApplication::instance()->applicationName().c_str());
}

void LinuxWaylandPlatformWindow::updateScaleFactor()
{
    // use the highest scale factor of all the outputs this window is visible on
    float factor = 1;
    for (auto *output : m_enteredOutputs) {
        if (output->scaleFactor() > factor) {
            factor = output->scaleFactor();
        }
    }
    window()->scaleFactor = factor;
}
