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

#include <xcb/xcb.h>

namespace KDGui {

class LinuxXcbPlatformIntegration;

class LinuxXcbClipboard : public AbstractClipboard
{
public:
    LinuxXcbClipboard(KDGui::LinuxXcbPlatformIntegration *integration);
    ~LinuxXcbClipboard();

    std::string text() override;
    void setText(std::string_view text) override;

private:
    KDGui::LinuxXcbPlatformIntegration *m_integration{ nullptr };
    xcb_atom_t m_clipboard, m_utf8;
    uint32_t m_window;
};

} // namespace KDGui
