/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_platform_window.h>
#include <KDGui/kdgui_global.h>
#include <kdbindings/signal.h>

struct wl_array;
struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;
struct wl_output;
struct wl_display;

namespace KDGui {

class LinuxWaylandPlatformIntegration;
class LinuxWaylandPlatformOutput;

class KDGUI_API LinuxWaylandPlatformWindow : public AbstractPlatformWindow
{
public:
    explicit LinuxWaylandPlatformWindow(LinuxWaylandPlatformIntegration *platformIntegraton,
                                        Window *window);

    LinuxWaylandPlatformWindow() = delete;

    LinuxWaylandPlatformWindow(LinuxWaylandPlatformWindow const &other) = delete;
    LinuxWaylandPlatformWindow &operator=(LinuxWaylandPlatformWindow const &other) = delete;

    LinuxWaylandPlatformWindow(LinuxWaylandPlatformWindow &&other) noexcept = default;
    LinuxWaylandPlatformWindow &operator=(LinuxWaylandPlatformWindow &&other) noexcept = default;

    static LinuxWaylandPlatformWindow *fromSurface(wl_surface *surface);
    inline wl_surface *surface() const { return m_surface; }
    wl_display *display() const;

    bool create() override;
    bool destroy() override;
    bool isCreated() override { return m_surface != nullptr; }

    void map() override;
    void unmap() override;

    void disableCursor() override;
    void enableCursor() override;

    void enableRawMouseInput() override;
    void disableRawMouseInput() override;

    void setTitle(const std::string &title) override;

    void setSize(uint32_t width, uint32_t height) override;
    void handleResize(uint32_t width, uint32_t height) override;

    void handleMousePress(
            uint32_t timestamp, MouseButtons buttons,
            int16_t xPos, int16_t yPos) override;

    void handleMouseRelease(
            uint32_t timestamp, MouseButtons buttons,
            int16_t xPos, int16_t yPos) override;

    void handleMouseMove(
            uint32_t timestamp, MouseButtons buttons,
            int64_t xPos, int64_t yPos) override;
    void handleMouseMoveRelative(uint32_t timestamp, int64_t dx, int64_t dy);

    void handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta) override;

    void handleKeyPress(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers) override;
    void handleKeyRelease(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers) override;
    void handleTextInput(const std::string &str) override;

    KDBindings::Signal<> cursorChanged;

    wl_surface *handle() const { return m_surface; }

private:
    enum class CursorMode {
        Normal = 0,
        Disabled = 1
    };

    void enter(wl_surface *surface, wl_output *output);
    void leave(wl_surface *surface, wl_output *output);

    void configure(xdg_surface *xdgSurface, uint32_t serial);
    void configureToplevel(xdg_toplevel *toplevel, int32_t width, int32_t height, wl_array *states);
    void close(xdg_toplevel *toplevel);
    void configureBounds(xdg_toplevel *toplevel, int32_t width, int32_t height);

    void setAppId();
    void updateScaleFactor();

    int32_t scaleByFactor(int32_t value) const;

    LinuxWaylandPlatformIntegration *m_platformIntegration;
    wl_surface *m_surface{ nullptr };
    xdg_surface *m_xdgSurface{ nullptr };
    xdg_toplevel *m_toplevel{ nullptr };
    CursorMode m_cursorMode{ CursorMode::Normal };
    std::vector<LinuxWaylandPlatformOutput *> m_enteredOutputs;
};

} // namespace KDGui
