/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cstdint>
#include <string>

#include <KDGui/abstract_clipboard.h>

struct wl_surface;
struct wl_seat;
struct wl_data_device;
struct wl_data_offer;
using wl_fixed_t = int32_t;

namespace KDGui {

class LinuxWaylandPlatformIntegration;

class LinuxWaylandClipboard : public AbstractClipboard
{
public:
    LinuxWaylandClipboard(KDGui::LinuxWaylandPlatformIntegration *integration);

    void initializeDataDevice(wl_seat *seat);

    std::string text() override;
    void setText(std::string_view text) override;

private:
    void dataOffer(wl_data_device *data_device, wl_data_offer *offer);
    void dragEnter(wl_data_device *wl_data_device, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer *id);
    void dragLeave(wl_data_device *wl_data_device);
    void dragMotion(wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    void dragDrop(wl_data_device *wl_data_device);
    void newSelection(wl_data_device *wl_data_device, wl_data_offer *id);

    wl_data_device *m_dataDevice{ nullptr };
    wl_data_offer *m_dataOffer{ nullptr };
    KDGui::LinuxWaylandPlatformIntegration *m_integration{ nullptr };
};

} // namespace KDGui
