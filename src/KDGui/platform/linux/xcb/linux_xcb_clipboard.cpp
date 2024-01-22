/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xcb_clipboard.h"

#include "linux_xcb_platform_integration.h"

KDGui::LinuxXcbClipboard::LinuxXcbClipboard(KDGui::LinuxXcbPlatformIntegration *integration)
    : m_integration{ integration }
{
    auto connection = m_integration->connection();

    // Create a dummy window that will be dedicated to clipboard I/O
    m_window = xcb_generate_id(connection);

    xcb_create_window(
            connection,
            XCB_COPY_FROM_PARENT,
            m_window,
            m_integration->screen()->root,
            0, 0,
            1,
            1,
            10,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            m_integration->screen()->root_visual,
            0,
            nullptr);

    // Get the CLIPBOARD atom, which holds the clipboard data
    const xcb_intern_atom_cookie_t selCookie = xcb_intern_atom(connection, false, 9, "CLIPBOARD");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, selCookie, nullptr);
    m_clipboard = reply->atom;

    // Get the UTF8_STRING atom, which is used to exchange UTF8 data between clients
    const xcb_intern_atom_cookie_t utf8Cookie = xcb_intern_atom(connection, false, 11, "UTF8_STRING");
    reply = xcb_intern_atom_reply(connection, utf8Cookie, nullptr);
    m_utf8 = reply->atom;
}

KDGui::LinuxXcbClipboard::~LinuxXcbClipboard()
{
    xcb_destroy_window(m_integration->connection(), m_window);
}

std::string KDGui::LinuxXcbClipboard::text()
{
    auto connection = m_integration->connection();

    const xcb_get_selection_owner_cookie_t ownerCookie = xcb_get_selection_owner(m_integration->connection(), m_clipboard);
    auto ownerReply = xcb_get_selection_owner_reply(connection, ownerCookie, nullptr);

    // No one has any clipboard data
    if (ownerReply == nullptr || ownerReply->owner == 0) {
        return "";
    }

    xcb_convert_selection(connection, m_window, m_clipboard, m_utf8, m_clipboard, XCB_TIME_CURRENT_TIME);
    xcb_flush(connection);

    // Await the selection reply from the other client
    for (;;) {
        if (auto xcbEvent = xcb_poll_for_event(connection)) {
            const auto eventType = xcbEvent->response_type & 0x7f;

            if (eventType == XCB_SELECTION_NOTIFY) {
                break;
            }
        }
    }

    auto propertyCookie = xcb_get_property(connection, 1, m_window, m_clipboard, m_utf8, 0, 0);
    auto propertyReply = xcb_get_property_reply(connection, propertyCookie, nullptr);

    const uint32_t ret = propertyReply->bytes_after;
    if (ret == 0) {
        return "";
    }

    propertyCookie = xcb_get_property(connection, 1, m_window, m_clipboard, m_utf8, 0, (ret + 4) >> 2);
    propertyReply = xcb_get_property_reply(connection, propertyCookie, nullptr);

    const int clipboardLength = xcb_get_property_value_length(propertyReply);
    if (clipboardLength == 0) {
        return "";
    }

    std::vector<char> buffer(clipboardLength);
    std::copy_n(reinterpret_cast<char *>(xcb_get_property_value(propertyReply)), clipboardLength, buffer.data());

    return buffer.data();
}

void KDGui::LinuxXcbClipboard::setText(std::string_view text)
{
    // TODO: xcb stub
}
