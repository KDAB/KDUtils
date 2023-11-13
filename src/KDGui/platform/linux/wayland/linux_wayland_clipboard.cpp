/*
    This file is part of KDUtils.

    SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: MIT

    Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_wayland_clipboard.h"

#include <wayland-client-protocol.h>
#include <unistd.h>

#include "linux_wayland_platform_integration.h"

using namespace KDGui;

void LinuxWaylandClipboard::initializeDataDevice(wl_seat *seat)
{
    static const struct wl_data_device_listener listener = {
        wrapWlCallback<&LinuxWaylandClipboard::dataOffer>,
        wrapWlCallback<&LinuxWaylandClipboard::dragEnter>,
        wrapWlCallback<&LinuxWaylandClipboard::dragLeave>,
        wrapWlCallback<&LinuxWaylandClipboard::dragMotion>,
        wrapWlCallback<&LinuxWaylandClipboard::dragDrop>,
        wrapWlCallback<&LinuxWaylandClipboard::newSelection>
    };

    m_dataDevice = wl_data_device_manager_get_data_device(m_integration->dataDeviceManager().object, seat);

    wl_data_device_add_listener(m_dataDevice, &listener, this);
}

std::string LinuxWaylandClipboard::text()
{
    // If there's no data source to copy from, skip
    if (m_dataOffer == nullptr) {
        return "";
    }

    // Open up two file descriptors using pipe().
    // The second fd is for writing (used by the data source, so we close it immediately.)
    // The first fd is used for reading back the data.
    std::array<int, 2> fds;
    if (pipe(fds.data()) < 0) {
        SPDLOG_LOGGER_ERROR(m_integration->logger(), "Failed to create pipe for clipboard!");
        return "";
    }
    wl_data_offer_receive(m_dataOffer, "text/plain", fds[1]);
    close(fds[1]);

    wl_display_roundtrip(m_integration->display());

    std::string buffer;
    while (true) {
        std::array<char, 1024> buf;
        const ssize_t n = read(fds[0], buf.data(), buf.size());
        if (n <= 0) {
            break;
        }

        buffer.append(buf.data(), n);
    }

    close(fds[0]);

    return buffer;
}

void KDGui::LinuxWaylandClipboard::setText(std::string_view text)
{
    // TODO: wayland stub
}

LinuxWaylandClipboard::LinuxWaylandClipboard(KDGui::LinuxWaylandPlatformIntegration *integration)
    : m_integration{ integration }
{
}

void LinuxWaylandClipboard::dataOffer(wl_data_device *data_device, wl_data_offer *offer)
{
    m_dataOffer = offer;
}

void LinuxWaylandClipboard::dragEnter(wl_data_device *wl_data_device, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id)
{
}

void LinuxWaylandClipboard::dragLeave(wl_data_device *wl_data_device)
{
}

void LinuxWaylandClipboard::dragMotion(wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

void LinuxWaylandClipboard::dragDrop(wl_data_device *wl_data_device)
{
}

void LinuxWaylandClipboard::newSelection(wl_data_device *wl_data_device, wl_data_offer *id)
{
}
