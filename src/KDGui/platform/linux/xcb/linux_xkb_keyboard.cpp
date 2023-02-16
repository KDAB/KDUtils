/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xkb_keyboard.h"
#include "linux_xcb_platform_integration.h"
#include "KDGui/platform/linux/common/linux_xkb.h"
#include "KDGui/platform/linux/common/linux_xkb_keyboard_map.h"

#include <xkbcommon/xkbcommon-x11.h>

#define explicit i_am_not_really_using_explicit
#include <xcb/xkb.h>
#undef explicit

using namespace KDGui;

LinuxXkbKeyboard::LinuxXkbKeyboard(LinuxXcbPlatformIntegration *platformIntegration)
    : m_platformIntegration{ platformIntegration }
{
    m_logger = spdlog::get("xkeyboard");
    if (!m_logger) {
        m_logger = spdlog::stdout_color_mt("xkeyboard");
        m_logger->set_level(spdlog::level::trace);
    }

    // Query the core keyboard device id
    m_deviceId = xkb_x11_get_core_keyboard_device_id(m_platformIntegration->connection());
    if (m_deviceId == -1) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to obtain core keyboard device id! Aborting.");
        throw std::runtime_error("Failed to obtain core keyboard device id!");
    }

    // Register for the event types we wish to receive
    registerForEvents();

    // Obtain an xkbcommon context
    m_context.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
    if (!m_context) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to obtain xkb context! Aborting.");
        throw std::runtime_error("Failed to obtain xkb context!");
    }

    // Obtain a keymap
    m_keymap.reset(
            xkb_x11_keymap_new_from_device(
                    m_context.get(),
                    m_platformIntegration->connection(),
                    m_deviceId,
                    XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (!m_keymap) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to obtain xkb keymap! Aborting.");
        throw std::runtime_error("Failed to obtain xkb keymap!");
    }

    // Obtain the keyboard state
    m_state.reset(
            xkb_x11_state_new_from_device(
                    m_keymap.get(),
                    m_platformIntegration->connection(),
                    m_deviceId));
    if (!m_state) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to obtain xkb state! Aborting.");
        throw std::runtime_error("Failed to obtain xkb state!");
    }
}

LinuxXkbKeyboard::~LinuxXkbKeyboard() = default;

void LinuxXkbKeyboard::registerForEvents()
{
    constexpr const auto requiredEvents = (XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
                                           XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
                                           XCB_XKB_EVENT_TYPE_STATE_NOTIFY);

    constexpr const auto requiredNknDetails = XCB_XKB_NKN_DETAIL_KEYCODES;

    constexpr const auto requiredMapParts = (XCB_XKB_MAP_PART_KEY_TYPES |
                                             XCB_XKB_MAP_PART_KEY_SYMS |
                                             XCB_XKB_MAP_PART_MODIFIER_MAP |
                                             XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
                                             XCB_XKB_MAP_PART_KEY_ACTIONS |
                                             XCB_XKB_MAP_PART_VIRTUAL_MODS |
                                             XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP);

    constexpr const auto requiredStateDetails = (XCB_XKB_STATE_PART_MODIFIER_BASE |
                                                 XCB_XKB_STATE_PART_MODIFIER_LATCH |
                                                 XCB_XKB_STATE_PART_MODIFIER_LOCK |
                                                 XCB_XKB_STATE_PART_GROUP_BASE |
                                                 XCB_XKB_STATE_PART_GROUP_LATCH |
                                                 XCB_XKB_STATE_PART_GROUP_LOCK);

    static constexpr const xcb_xkb_select_events_details_t details = {
        .affectNewKeyboard = requiredNknDetails,
        .newKeyboardDetails = requiredNknDetails,
        .affectState = requiredStateDetails,
        .stateDetails = requiredStateDetails
    };

    auto cookie = xcb_xkb_select_events_aux_checked(
            m_platformIntegration->connection(),
            m_deviceId,
            requiredEvents, /* affectWhich */
            0, /* clear */
            0, /* selectAll */
            requiredMapParts, /* affectMap */
            requiredMapParts, /* map */
            &details);

    xcb_generic_error_t *error = xcb_request_check(
            m_platformIntegration->connection(), cookie);

    if (error) {
        free(error);
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to register for xkeyboard events! Aborting.");
        throw std::runtime_error("Failed to register for xkeyboard events");
    }
}

void LinuxXkbKeyboard::handleKeyPress(xcb_key_press_event_t *ev)
{
    auto window = m_platformIntegration->window(ev->event);
    SPDLOG_LOGGER_DEBUG(m_logger, "Key press event for key {}", ev->detail);

    // Fetch the keysym of the pressed key
    const xkb_keycode_t keycode = ev->detail;
    const auto keysym = xkb_state_key_get_one_sym(m_state.get(), keycode);

    // Update the xkb state
    (void)xkb_state_update_key(m_state.get(), keycode, XKB_KEY_DOWN);

    char s[16];
    xkb_keysym_get_name(keysym, s, sizeof(s));
    SPDLOG_LOGGER_TRACE(m_logger, "keysym: {} => name: {}", keysym, s);

    // Lookup the KDGui key enum
    auto key = xkb::keysymToKey(keysym);

    // Get the modifier state
    const auto modifiers = xkb::modifierState(m_state.get());

    // Generate a key press event
    window->handleKeyPress(ev->time, ev->detail, key, modifiers);

    // Get the unicode characters
    xkb_state_key_get_utf8(m_state.get(), keycode, s, sizeof(s));
    if (strlen(s) > 0)
        window->handleTextInput(s);
}

void LinuxXkbKeyboard::handleKeyRelease(xcb_key_release_event_t *ev)
{
    auto window = m_platformIntegration->window(ev->event);
    SPDLOG_LOGGER_DEBUG(m_logger, "Key release event for key {}", ev->detail);

    // Fetch the keysym of the released key
    const xkb_keycode_t keycode = ev->detail;
    const auto keysym = xkb_state_key_get_one_sym(m_state.get(), keycode);

    // Update the xkb state
    (void)xkb_state_update_key(m_state.get(), keycode, XKB_KEY_UP);

    char s[16];
    xkb_keysym_get_name(keysym, s, sizeof(s));
    SPDLOG_LOGGER_TRACE(m_logger, "keysym: {} => name: {}", keysym, s);

    // Lookup the KDGui key enum
    auto key = xkb::keysymToKey(keysym);

    // Get the modifier state
    const auto modifiers = xkb::modifierState(m_state.get());

    // Generate a key release event
    window->handleKeyRelease(ev->time, ev->detail, key, modifiers);
}
