/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cstdint>
#include <string>

struct wl_output;

namespace KDGui {

class LinuxWaylandPlatformOutput
{
public:
    static constexpr uint32_t supportedVersion = 3;

    explicit LinuxWaylandPlatformOutput(wl_output *output, uint32_t version, uint32_t id);
    LinuxWaylandPlatformOutput(const LinuxWaylandPlatformOutput &) = delete;
    ~LinuxWaylandPlatformOutput();

    static LinuxWaylandPlatformOutput *fromOutput(wl_output *output);

    uint32_t outputId() const { return m_outputId; }
    int scaleFactor() const { return m_scaleFactor; }

private:
    void geometry(wl_output *output, int32_t x, int32_t y, int32_t physicalWidth, int32_t physicalHeight,
                  int32_t subpixel, const char *make, const char *model, int32_t transform);
    void mode(wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate);
    void done(wl_output *output);
    void scale(wl_output *output, int32_t factor);

    wl_output *m_output;
    uint32_t m_version;
    uint32_t m_outputId;
    int m_scaleFactor{ 1 };
    std::string m_make;
    std::string m_model;
};

} // namespace KDGui
