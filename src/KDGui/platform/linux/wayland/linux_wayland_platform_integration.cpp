/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_wayland_platform_integration.h"

#include <KDFoundation/core_application.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_input.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_output.h>
#include <KDUtils/logging.h>

#include <wayland-client-core.h>
#include <wayland-cursor.h>
#include <wayland-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-zwp-relative-pointer-unstable-v1-client-protocol.h>
#include <wayland-zwp-pointer-constraints-v1-client-protocol.h>
#include <wayland-xdg-decoration-unstable-v1-client-protocol.h>

using namespace KDGui;

LinuxWaylandPlatformIntegration::LinuxWaylandPlatformIntegration()
    : m_clipboard{ new LinuxWaylandClipboard{ this } }
{
}

LinuxWaylandPlatformIntegration::~LinuxWaylandPlatformIntegration()
{
    m_outputs.clear();
    m_inputs.clear();

    xdg_wm_base_destroy(m_xdgShell.object);
    wl_compositor_destroy(m_compositor.object);
    wl_shm_destroy(m_shm.object);

    wl_registry_destroy(m_registry);
    wl_display_disconnect(m_display);
    SPDLOG_LOGGER_DEBUG(m_logger, "Destroyed wl_display");
}

void LinuxWaylandPlatformIntegration::init()
{
    m_logger = KDUtils::Logger::logger("wayland", spdlog::level::info);

    SPDLOG_LOGGER_INFO(m_logger, "Wayland display is: {}", getenv("WAYLAND_DISPLAY"));

    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to connect to the Wayland server! Aborting.");
        throw std::runtime_error("Failed to connect to the Wayland server!");
    }
    SPDLOG_LOGGER_INFO(m_logger, "Connected to the Wayland server");

    // Create a queue which we'll use for all incoming messages.
    // This is to ensure any library that may also be running in the executable, if
    // it talks to the wayland server, does not dispatch our messages behind our back.
    m_queue = wl_display_create_queue(m_display);

    // This proxy stuff may seem fishy with all these casts but it is actually correct.
    // All wayland protocol objects are really wl_proxy, they are just casted to a different
    // type.
    // By creating a proxy for the wl_display and by setting the queue on the proxy, all
    // objects created from it will inherit the queue in a thread safe manner.
    m_displayProxy = static_cast<wl_display *>(wl_proxy_create_wrapper(m_display));
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_displayProxy), m_queue);

    m_registry = wl_display_get_registry(m_displayProxy);
    static const wl_registry_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformIntegration::global>,
        wrapWlCallback<&LinuxWaylandPlatformIntegration::globalRemove>
    };
    wl_registry_add_listener(m_registry, &listener, this);
    wl_display_roundtrip_queue(m_display, m_queue);

    if (!m_compositor.object) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server does not support wl_compositor! Aborting.");
        throw std::runtime_error("The Wayland server does not support wl_compositor!");
    }
    if (!m_xdgShell.object) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server does not support xdg_wm_base! Aborting.");
        throw std::runtime_error("The Wayland server does not support xdg_wm_base!");
    }
    if (!m_shm.object) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server does not support wl_shm! Aborting.");
        throw std::runtime_error("The Wayland server does not support wl_shm!");
    }

    m_cursorTheme = wl_cursor_theme_load(nullptr, 24, m_shm.object);
    static_cast<LinuxWaylandPlatformEventLoop *>(KDFoundation::CoreApplication::instance()->eventLoop())->init();
}

bool LinuxWaylandPlatformIntegration::checkAvailable()
{
    auto d = wl_display_connect(nullptr);
    if (d) {
        wl_display_disconnect(d);
        return true;
    }
    return false;
}

AbstractClipboard *LinuxWaylandPlatformIntegration::clipboard()
{
    return m_clipboard.get();
}

LinuxWaylandPlatformEventLoop *LinuxWaylandPlatformIntegration::createPlatformEventLoopImpl()
{
    return new LinuxWaylandPlatformEventLoop(this);
}

LinuxWaylandPlatformWindow *LinuxWaylandPlatformIntegration::createPlatformWindowImpl(Window *window)
{
    return new LinuxWaylandPlatformWindow(this, window);
}

void LinuxWaylandPlatformIntegration::global(wl_registry *registry, uint32_t id, const char *name, uint32_t version)
{
    if (std::string_view{ wl_compositor_interface.name } == name) {
        auto ver = std::min(4u, version);
        auto compositor = reinterpret_cast<wl_compositor *>(wl_registry_bind(registry, id, &wl_compositor_interface, ver));
        m_compositor = { compositor, ver, id };
    } else if (std::string_view{ wl_shm_interface.name } == name) {
        auto shm = reinterpret_cast<wl_shm *>(wl_registry_bind(registry, id, &wl_shm_interface, 1));
        m_shm = { shm, 1, id };
    } else if (std::string_view{ xdg_wm_base_interface.name } == name) {
        auto ver = std::min(2u, version);
        auto xdgShell = reinterpret_cast<xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, ver));
        m_xdgShell = { xdgShell, ver, id };

        static const xdg_wm_base_listener listener = {
            wrapWlCallback<&LinuxWaylandPlatformIntegration::ping>
        };
        xdg_wm_base_add_listener(xdgShell, &listener, this);
    } else if (std::string_view{ wl_seat_interface.name } == name) {
        auto ver = std::min(LinuxWaylandPlatformInput::supportedVersion, version);
        auto seat = reinterpret_cast<wl_seat *>(wl_registry_bind(registry, id, &wl_seat_interface, ver));
        m_inputs.push_back(std::make_unique<LinuxWaylandPlatformInput>(this, seat, ver, id));
    } else if (std::string_view{ wl_output_interface.name } == name) {
        auto ver = std::min(LinuxWaylandPlatformOutput::supportedVersion, version);
        auto output = reinterpret_cast<wl_output *>(wl_registry_bind(registry, id, &wl_output_interface, ver));
        m_outputs.push_back(std::make_unique<LinuxWaylandPlatformOutput>(output, ver, id));
    } else if (std::string_view{ zwp_relative_pointer_manager_v1_interface.name } == name) {
        auto ver = std::min(LinuxWaylandPlatformInput::relativePointerV1SupportedVersion, version);
        auto manager = reinterpret_cast<zwp_relative_pointer_manager_v1 *>(wl_registry_bind(registry, id, &zwp_relative_pointer_manager_v1_interface, ver));
        m_relativePointerV1 = { manager, ver, id };
    } else if (std::string_view{ zwp_pointer_constraints_v1_interface.name } == name) {
        auto ver = std::min(LinuxWaylandPlatformInput::pointerConstraintsV1SupportedVersion, version);
        auto constraints = reinterpret_cast<zwp_pointer_constraints_v1 *>(wl_registry_bind(registry, id, &zwp_pointer_constraints_v1_interface, ver));
        m_pointerConstraintsV1 = { constraints, ver, id };
    } else if (std::string_view{ zxdg_decoration_manager_v1_interface.name } == name) {
        auto decoration = reinterpret_cast<zxdg_decoration_manager_v1 *>(wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
        m_decorationV1 = { decoration, 1, id };
    } else if (std::string_view{ wl_data_device_manager_interface.name } == name) {
        auto deviceManager = reinterpret_cast<wl_data_device_manager *>(wl_registry_bind(registry, id, &wl_data_device_manager_interface, 3));
        m_dataDeviceManager = { deviceManager, 1, id };

        for (auto &input : m_inputs) {
            m_clipboard->initializeDataDevice(input->seat());
        }
    }
}

void LinuxWaylandPlatformIntegration::globalRemove(wl_registry *registry, uint32_t id)
{
    auto findSeat = [this](uint32_t id) {
        return std::find_if(m_inputs.begin(), m_inputs.end(), [id](const auto &input) { return input->seatId() == id; });
    };

    auto findOutput = [this](uint32_t id) {
        return std::find_if(m_outputs.begin(), m_outputs.end(), [id](const auto &output) { return output->outputId() == id; });
    };

    if (id == m_compositor.id) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server removed the wl_compositor global! Aborting.");
        throw std::runtime_error("The Wayland server removed the wl_compositor globa!");
    } else if (id == m_xdgShell.id) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server removed the xdg_wm_base global! Aborting.");
        throw std::runtime_error("The Wayland server removed the xdg_wm_base globa!");
    } else if (id == m_shm.id) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "The Wayland server removed the wl_shm global! Aborting.");
        throw std::runtime_error("The Wayland server removed the wl_shm globa!");
    } else if (auto it = findSeat(id); it != m_inputs.end()) {
        m_inputs.erase(it);
    } else if (auto it = findOutput(id); it != m_outputs.end()) {
        m_outputs.erase(it);
    } else if (id == m_relativePointerV1.id) {
        for (auto &i : m_inputs) {
            i->destroyRelativePointerV1();
        }
        zwp_relative_pointer_manager_v1_destroy(m_relativePointerV1.object);
    } else if (id == m_pointerConstraintsV1.id) {
        for (auto &i : m_inputs) {
            i->destroyPointerConstraintsV1();
        }
        zwp_pointer_constraints_v1_destroy(m_pointerConstraintsV1.object);
    }
}

void LinuxWaylandPlatformIntegration::ping(xdg_wm_base *xdgShell, uint32_t serial)
{
    xdg_wm_base_pong(xdgShell, serial);
}
