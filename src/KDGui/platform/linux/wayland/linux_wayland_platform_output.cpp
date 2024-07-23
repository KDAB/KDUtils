/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/platform/linux/wayland/linux_wayland_platform_output.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_integration.h>

#include <wayland-client-protocol.h>

using namespace KDGui;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
LinuxWaylandPlatformOutput::LinuxWaylandPlatformOutput(wl_output *output, uint32_t version, uint32_t id)
    : m_output(output)
    , m_version(version)
    , m_outputId(id)
{
    static const wl_output_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformOutput::geometry>,
        wrapWlCallback<&LinuxWaylandPlatformOutput::mode>,
        wrapWlCallback<&LinuxWaylandPlatformOutput::done>,
        wrapWlCallback<&LinuxWaylandPlatformOutput::scale>
    };
    wl_output_add_listener(output, &listener, this);
}

LinuxWaylandPlatformOutput::~LinuxWaylandPlatformOutput()
{
    if (m_version >= WL_OUTPUT_RELEASE_SINCE_VERSION) {
        wl_output_release(m_output);
    } else {
        wl_output_destroy(m_output);
    }
}

LinuxWaylandPlatformOutput *LinuxWaylandPlatformOutput::fromOutput(wl_output *output)
{
    return static_cast<LinuxWaylandPlatformOutput *>(wl_output_get_user_data(output));
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void LinuxWaylandPlatformOutput::geometry(wl_output * /*output*/, int32_t /*x*/, int32_t /*y*/, int32_t /*physicalWidth*/, int32_t /*physicalHeight*/,
                                          int32_t /*subpixel*/, const char *make, const char *model, int32_t /*transform*/)
{
    m_make = make;
    m_model = model;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void LinuxWaylandPlatformOutput::mode(wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate)
{
}

void LinuxWaylandPlatformOutput::done(wl_output *output)
{
}

void LinuxWaylandPlatformOutput::scale(wl_output * /*output*/, int32_t factor)
{
    m_scaleFactor = factor;
}
