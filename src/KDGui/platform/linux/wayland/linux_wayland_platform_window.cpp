/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_wayland_platform_window.h"
#include <KDGui/window.h>
#include "linux_wayland_platform_integration.h"
#include "linux_wayland_platform_output.h"

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-xdg-decoration-unstable-v1-client-protocol.h>

#include <array>
#include <cassert>
#include <tuple>

using namespace KDFoundation;
using namespace KDGui;

LinuxWaylandPlatformWindow::LinuxWaylandPlatformWindow(
        LinuxWaylandPlatformIntegration *platformIntegration,
        Window *window)
    : AbstractPlatformWindow(window, AbstractPlatformWindow::Type::Wayland)
    , m_platformIntegration{ platformIntegration }
{
    std::ignore = CoreApplication::instance()->applicationName.valueChanged().connect([this]() {
        if (m_toplevel) {
            setAppId();
        }
    });
    m_scaleChangedConnection = window->scaleFactor.valueChanged().connect([this](float value) {
        if (m_surface) {
            wl_surface_set_buffer_scale(m_surface, int32_t(value));
        }
    });
}

LinuxWaylandPlatformWindow *LinuxWaylandPlatformWindow::fromSurface(wl_surface *surface)
{
    auto w = static_cast<LinuxWaylandPlatformWindow *>(wl_surface_get_user_data(surface));
    assert(w);
    return w;
}

wl_display *LinuxWaylandPlatformWindow::display() const
{
    return m_platformIntegration->display();
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

    wl_surface_set_buffer_scale(m_surface, int32_t(window()->scaleFactor()));

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

    if (auto manager = m_platformIntegration->xdgDecoration().object) {
        auto decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(manager, m_toplevel);

        zxdg_toplevel_decoration_v1_set_mode(decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
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

void LinuxWaylandPlatformWindow::grabMouse()
{
}

void LinuxWaylandPlatformWindow::releaseMouse()
{
}

void LinuxWaylandPlatformWindow::setTitle(const std::string &title)
{
    xdg_toplevel_set_title(m_toplevel, title.c_str());
}

void LinuxWaylandPlatformWindow::setSize(uint32_t width, uint32_t height)
{
    if (m_xdgSurface) {
        xdg_surface_set_window_geometry(m_xdgSurface, 0, 0, int32_t(width), int32_t(height));
    }
}

void LinuxWaylandPlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    // Pass a resize event to the window
    ResizeEvent ev{ static_cast<uint32_t>(scaleByFactor(width)), static_cast<uint32_t>(scaleByFactor(height)) };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMousePress(uint32_t timestamp, MouseButton button,
                                                  int16_t xPos, int16_t yPos)
{
    m_mouseButtons.setFlag(button);
    MousePressEvent ev{ timestamp, button, m_mouseButtons, static_cast<int16_t>(scaleByFactor(xPos)), static_cast<int16_t>(scaleByFactor(yPos)) };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMouseRelease(uint32_t timestamp, MouseButton button,
                                                    int16_t xPos, int16_t yPos)
{
    m_mouseButtons.setFlag(button, false);
    MouseReleaseEvent ev{ timestamp, button, m_mouseButtons, static_cast<int16_t>(scaleByFactor(xPos)), static_cast<int16_t>(scaleByFactor(yPos)) };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void LinuxWaylandPlatformWindow::handleMouseMove(uint32_t timestamp, MouseButton /* button */,
                                                 int64_t x, int64_t y)
{
    if (m_cursorMode == CursorMode::Normal) {
        MouseMoveEvent ev{ timestamp, m_mouseButtons, scaleByFactor(x), scaleByFactor(y) };
        CoreApplication::instance()->sendEvent(m_window, &ev);
    }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void LinuxWaylandPlatformWindow::handleMouseMoveRelative(uint32_t timestamp, int64_t dx, int64_t dy)
{
    if (m_cursorMode == CursorMode::Disabled) {
        const Position pos = window()->cursorPosition.get() + Position(dx, dy);
        MouseMoveEvent ev{ timestamp, MouseButton::NoButton, pos.x, pos.y };
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

void LinuxWaylandPlatformWindow::enter(wl_surface * /*surface*/, wl_output *output)
{
    auto o = LinuxWaylandPlatformOutput::fromOutput(output);
    m_enteredOutputs.push_back(o);

    updateScaleFactor();
}

void LinuxWaylandPlatformWindow::leave(wl_surface * /*surface*/, wl_output *output)
{
    auto o = LinuxWaylandPlatformOutput::fromOutput(output);
    auto it = std::find(m_enteredOutputs.begin(), m_enteredOutputs.end(), o);
    if (it != m_enteredOutputs.end()) {
        m_enteredOutputs.erase(it);
    }

    updateScaleFactor();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void LinuxWaylandPlatformWindow::configure(xdg_surface *xdgSurface, uint32_t serial)
{
    xdg_surface_ack_configure(xdgSurface, serial);
}

void LinuxWaylandPlatformWindow::configureToplevel(xdg_toplevel * /*toplevel*/, int32_t width, int32_t height, wl_array * /*states*/)
{
    if (width != 0 && height != 0) {
        handleResize(width, height);
    }
}

void LinuxWaylandPlatformWindow::close(xdg_toplevel * /*toplevel*/)
{
    SPDLOG_LOGGER_DEBUG(m_platformIntegration->logger(), "Window closed by user");
    window()->visible = false;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void LinuxWaylandPlatformWindow::configureBounds(xdg_toplevel * /*toplevel*/, int32_t width, int32_t height)
{
    auto w = int32_t(window()->width());
    auto h = int32_t(window()->height());
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
        const auto outputScaleFactor = float(output->scaleFactor());
        if (outputScaleFactor > factor) {
            factor = outputScaleFactor;
        }
    }
    window()->scaleFactor = factor;
}

template<typename T>
T LinuxWaylandPlatformWindow::scaleByFactor(T value) const
{
    return value * m_window->scaleFactor.get(); // NOLINT(bugprone-narrowing-conversions)
}
