/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/platform/linux/wayland/linux_wayland_platform_input.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_integration.h>
#include <KDGui/platform/linux/wayland/linux_wayland_platform_window.h>
#include <KDGui/platform/linux/common/linux_xkb_keyboard_map.h>
#include <KDGui/platform/linux/common/linux_xkb.h>
#include <KDGui/window.h>

#include <unistd.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>
#include <wayland-cursor.h>
#include <wayland-client-protocol.h>
#include <wayland-zwp-relative-pointer-unstable-v1-client-protocol.h>
#include <wayland-zwp-pointer-constraints-v1-client-protocol.h>

using namespace KDGui;

LinuxWaylandPlatformInput::LinuxWaylandPlatformInput(LinuxWaylandPlatformIntegration *integration,
                                                     wl_seat *seat, uint32_t version, uint32_t id)
    : m_integration(integration)
    , m_seat(seat)
    , m_version(version)
    , m_seatId(id)
{
    static const wl_seat_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformInput::capabilities>,
        wrapWlCallback<&LinuxWaylandPlatformInput::name>
    };
    wl_seat_add_listener(seat, &listener, this);

    m_keyboard.repeat.timer.timeout.connect(&LinuxWaylandPlatformInput::keyboardRepeatKey, this);
}

LinuxWaylandPlatformInput::~LinuxWaylandPlatformInput()
{
    if (m_pointer.pointer) {
        destroyPointer();
    }
    if (m_keyboard.keyboard) {
        destroyKeyboard();
    }
    if (m_touch.touch) {
        destroyTouch();
    }

    if (m_version >= WL_SEAT_RELEASE_SINCE_VERSION) {
        wl_seat_release(m_seat);
    } else {
        wl_seat_destroy(m_seat);
    }
}

void LinuxWaylandPlatformInput::destroyRelativePointerV1()
{
    if (m_pointer.relativePointerV1) {
        zwp_relative_pointer_v1_destroy(m_pointer.relativePointerV1);
        m_pointer.relativePointerV1 = nullptr;
    }
}

void LinuxWaylandPlatformInput::destroyPointerConstraintsV1()
{
    if (m_pointer.lockedPointerV1) {
        zwp_locked_pointer_v1_destroy(m_pointer.lockedPointerV1);
        m_pointer.lockedPointerV1 = nullptr;
    }
}

void LinuxWaylandPlatformInput::capabilities(wl_seat *seat, uint32_t caps)
{
    const bool hasPointer = caps & WL_SEAT_CAPABILITY_POINTER;
    if (hasPointer && !m_pointer.pointer) {
        initPointer();
    } else if (hasPointer && m_pointer.pointer) {
        destroyPointer();
    }

    const bool hasKeyboard = caps & WL_SEAT_CAPABILITY_KEYBOARD;
    if (hasKeyboard && !m_keyboard.keyboard) {
        initKeyboard();
    } else if (!hasKeyboard && m_keyboard.keyboard) {
        destroyKeyboard();
    }

    const bool hasTouch = caps = WL_SEAT_CAPABILITY_TOUCH;
    if (hasTouch && !m_touch.touch) {
        initTouch();
    } else if (!hasTouch && m_touch.touch) {
        destroyTouch();
    }
}

void LinuxWaylandPlatformInput::name(wl_seat *seat, const char *name)
{
    m_name = name;
}

void LinuxWaylandPlatformInput::initPointer()
{
    m_pointer.pointer = wl_seat_get_pointer(m_seat);
    static const wl_pointer_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerEnter>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerLeave>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerMotion>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerButton>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerAxis>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerFrame>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerAxisSource>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerAxisStop>,
        wrapWlCallback<&LinuxWaylandPlatformInput::pointerAxisDiscrete>
    };
    wl_pointer_add_listener(m_pointer.pointer, &listener, this);

    m_pointer.cursorSurface = wl_compositor_create_surface(m_integration->compositor().object);

    if (auto manager = m_integration->relativePointerManagerV1().object) {
        m_pointer.relativePointerV1 = zwp_relative_pointer_manager_v1_get_relative_pointer(manager, m_pointer.pointer);

        static const zwp_relative_pointer_v1_listener listener = {
            wrapWlCallback<&LinuxWaylandPlatformInput::pointerRelativeMotionV1>
        };
        zwp_relative_pointer_v1_add_listener(m_pointer.relativePointerV1, &listener, this);
    }
}

void LinuxWaylandPlatformInput::destroyPointer()
{
    destroyRelativePointerV1();
    destroyPointerConstraintsV1();

    // m_version is the version of the seat but the way wayland protocol works child
    // objects (wl_pointer) have the same version as their parent (wl_seat) so this
    // is fine
    if (m_version >= WL_POINTER_RELEASE_SINCE_VERSION) {
        wl_pointer_release(m_pointer.pointer);
    } else {
        wl_pointer_destroy(m_pointer.pointer);
    }
    m_pointer.pointer = nullptr;
    m_pointer.focus = nullptr;
}

void LinuxWaylandPlatformInput::initKeyboard()
{
    m_keyboard.keyboard = wl_seat_get_keyboard(m_seat);
    static const wl_keyboard_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformInput::keyboardKeymap>,
        wrapWlCallback<&LinuxWaylandPlatformInput::keyboardEnter>,
        wrapWlCallback<&LinuxWaylandPlatformInput::keyboardLeave>,
        wrapWlCallback<&LinuxWaylandPlatformInput::keybordKey>,
        wrapWlCallback<&LinuxWaylandPlatformInput::keyboardModifiers>,
        wrapWlCallback<&LinuxWaylandPlatformInput::keyboardRepeatInfo>
    };
    wl_keyboard_add_listener(m_keyboard.keyboard, &listener, this);

    m_keyboard.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
}

void LinuxWaylandPlatformInput::destroyKeyboard()
{
    xkb_state_unref(m_keyboard.state);
    xkb_keymap_unref(m_keyboard.keymap);
    xkb_context_unref(m_keyboard.context);
    m_keyboard.context = nullptr;
    m_keyboard.keymap = nullptr;
    m_keyboard.state = nullptr;

    if (m_version >= WL_KEYBOARD_RELEASE_SINCE_VERSION) {
        wl_keyboard_release(m_keyboard.keyboard);
    } else {
        wl_keyboard_destroy(m_keyboard.keyboard);
    }
    m_keyboard.keyboard = nullptr;
    m_keyboard.focus = nullptr;
    m_keyboard.repeat.timer.running = false;
}

void LinuxWaylandPlatformInput::initTouch()
{
    m_touch.touch = wl_seat_get_touch(m_seat);
    static const wl_touch_listener listener = {
        wrapWlCallback<&LinuxWaylandPlatformInput::touchDown>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchUp>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchMotion>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchFrame>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchCancel>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchShape>,
        wrapWlCallback<&LinuxWaylandPlatformInput::touchOrientation>
    };
    wl_touch_add_listener(m_touch.touch, &listener, this);
}

void LinuxWaylandPlatformInput::destroyTouch()
{
    if (m_version >= WL_TOUCH_RELEASE_SINCE_VERSION) {
        wl_touch_release(m_touch.touch);
    } else {
        wl_touch_destroy(m_touch.touch);
    }
    m_touch.touch = nullptr;
}

void LinuxWaylandPlatformInput::pointerEnter(wl_pointer *pointer, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
    m_pointer.pos = { wl_fixed_from_int(x), wl_fixed_from_int(y) };
    m_pointer.lastSerial = serial;

    if (m_version >= WL_POINTER_FRAME_SINCE_VERSION) {
        m_pointer.accumulatedEvent.focus = LinuxWaylandPlatformWindow::fromSurface(surface);
        m_pointer.accumulatedEvent.focusChange = true;
    } else {
        m_pointer.focus = LinuxWaylandPlatformWindow::fromSurface(surface);
        m_pointer.focus->cursorChanged.connect(&LinuxWaylandPlatformInput::setCursor, this);
        setCursor();
    }
}

void LinuxWaylandPlatformInput::setCursor()
{
    if (m_pointer.focus->window()->cursorEnabled.get()) {
        pointerUnlock();

        // TODO support other cursors
        auto *cursor = wl_cursor_theme_get_cursor(m_integration->cursorTheme(), "left_ptr");

        // just get the first image, we don't care about animated cursors
        auto image = cursor->images[0];
        auto buffer = wl_cursor_image_get_buffer(image);

        wl_surface_attach(m_pointer.cursorSurface, buffer, 0, 0);
        wl_surface_commit(m_pointer.cursorSurface);

        wl_pointer_set_cursor(m_pointer.pointer, m_pointer.lastSerial, m_pointer.cursorSurface, image->hotspot_x, image->hotspot_y);
    } else {
        wl_pointer_set_cursor(m_pointer.pointer, m_pointer.lastSerial, nullptr, 0, 0);
        pointerLock();
    }
}

void LinuxWaylandPlatformInput::pointerUnlock()
{
    if (m_pointer.lockedPointerV1) {
        zwp_locked_pointer_v1_destroy(m_pointer.lockedPointerV1);
        m_pointer.lockedPointerV1 = nullptr;
    }
}

void LinuxWaylandPlatformInput::pointerLock()
{
    if (auto constraints = m_integration->pointerConstraintsV1().object; constraints && !m_pointer.lockedPointerV1) {
        m_pointer.lockedPointerV1 = zwp_pointer_constraints_v1_lock_pointer(constraints, m_pointer.focus->surface(),
                                                                            m_pointer.pointer, nullptr,
                                                                            ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
    }
}

void LinuxWaylandPlatformInput::pointerLeave(wl_pointer *pointer, uint32_t serial, wl_surface *surface)
{
    if (m_version >= WL_POINTER_FRAME_SINCE_VERSION) {
        m_pointer.accumulatedEvent.focus = nullptr;
        m_pointer.accumulatedEvent.focusChange = true;
    } else {
        m_pointer.focus->cursorChanged.disconnectAll();
        m_pointer.focus = nullptr;
    }
    m_pointer.lastSerial = serial;
}

void LinuxWaylandPlatformInput::pointerMotion(wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    m_pointer.pos = { wl_fixed_from_int(x), wl_fixed_from_int(y) };
    if (m_version >= WL_POINTER_FRAME_SINCE_VERSION) {
        m_pointer.accumulatedEvent.pos = m_pointer.pos;
        m_pointer.accumulatedEvent.time = time;
    } else {
        m_pointer.focus->handleMouseMove(time, 0, m_pointer.pos.x, m_pointer.pos.y);
    }
}

void LinuxWaylandPlatformInput::pointerButton(wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    const int btn = [=]() {
        switch (button) {
        case BTN_LEFT:
            return 1;
        case BTN_MIDDLE:
            return 2;
        case BTN_RIGHT:
            return 3;
        default:
            break;
        }
        return 0;
    }();

    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        m_pointer.focus->handleMousePress(time, btn, m_pointer.pos.x, m_pointer.pos.y);
    } else {
        m_pointer.focus->handleMouseRelease(time, btn, m_pointer.pos.x, m_pointer.pos.y);
    }
    m_pointer.lastSerial = serial;
}

void LinuxWaylandPlatformInput::pointerAxis(wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    const double delta = -wl_fixed_to_double(value);
    const int32_t xDelta = axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL ? delta : 0;
    const int32_t yDelta = axis == WL_POINTER_AXIS_VERTICAL_SCROLL ? delta : 0;

    if (m_version >= WL_POINTER_FRAME_SINCE_VERSION) {
        m_pointer.accumulatedEvent.axis += Position(xDelta, yDelta);
        m_pointer.accumulatedEvent.time = time;
    } else {
        m_pointer.focus->handleMouseWheel(time, xDelta, yDelta);
    }
}

void LinuxWaylandPlatformInput::pointerFrame(wl_pointer *pointer)
{
    auto &ev = m_pointer.accumulatedEvent;
    if (ev.focusChange && ev.focus) {
        m_pointer.focus = ev.focus;
        m_pointer.focus->cursorChanged.connect(&LinuxWaylandPlatformInput::setCursor, this);
        setCursor();
    }

    if (ev.axis != Position(0, 0)) {
        m_pointer.focus->handleMouseWheel(ev.time, ev.axis.x, ev.axis.y);
        ev.axis = Position(0, 0);
    }
    if (ev.pos != Position(0, 0)) {
        m_pointer.focus->handleMouseMove(ev.time, 0, ev.pos.x, ev.pos.y);
        ev.pos = Position(0, 0);
    }
    if (ev.delta != Position(0, 0)) {
        m_pointer.focus->handleMouseMoveRelative(ev.time, ev.delta.x, ev.delta.y);
        ev.delta = {};
    }

    if (ev.focusChange && !ev.focus) {
        m_pointer.focus->cursorChanged.disconnectAll();
        m_pointer.focus = nullptr;
    }
    ev.focusChange = false;
}

void LinuxWaylandPlatformInput::pointerAxisSource(wl_pointer *pointer, uint32_t axis)
{
}

void LinuxWaylandPlatformInput::pointerAxisStop(wl_pointer *pointer, uint32_t time, uint32_t axis)
{
}

void LinuxWaylandPlatformInput::pointerAxisDiscrete(wl_pointer *pointer, uint32_t axis, int32_t steps)
{
}

void LinuxWaylandPlatformInput::pointerRelativeMotionV1(zwp_relative_pointer_v1 *pointer, uint32_t utimeHi, uint32_t utimeLow,
                                                        wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dxUnaccel, wl_fixed_t dyUnaccel)
{
    Position pos{ wl_fixed_from_int(dx), wl_fixed_from_int(dy) };
    uint64_t utime = uint64_t(utimeHi) << 32 | utimeLow;
    uint32_t time = utime / 1000;

    if (m_version >= WL_POINTER_FRAME_SINCE_VERSION) {
        m_pointer.accumulatedEvent.delta = pos;
        m_pointer.accumulatedEvent.time = time;
    } else {
        m_pointer.focus->handleMouseMoveRelative(time, pos.x, pos.y);
    }
}

void LinuxWaylandPlatformInput::keyboardKeymap(wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    if (m_keyboard.keymap) {
        xkb_state_unref(m_keyboard.state);
        xkb_keymap_unref(m_keyboard.keymap);
        m_keyboard.keymap = nullptr;
        m_keyboard.state = nullptr;
    }

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        SPDLOG_LOGGER_ERROR(m_integration->logger(), "Unsupported keymap format '%u'!", format);
        return;
    }

    char *map = static_cast<char *>(mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (!map) {
        SPDLOG_LOGGER_ERROR(m_integration->logger(), "mmapping the keymap fd failed!");
        close(fd);
        return;
    }

    m_keyboard.keymap = xkb_keymap_new_from_string(m_keyboard.context, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map, size);
    close(fd);

    m_keyboard.state = xkb_state_new(m_keyboard.keymap);
}

void LinuxWaylandPlatformInput::keyboardEnter(wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    m_keyboard.focus = LinuxWaylandPlatformWindow::fromSurface(surface);
}

void LinuxWaylandPlatformInput::keyboardLeave(wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    m_keyboard.focus = nullptr;
    m_keyboard.repeat.timer.running = false;
}

void LinuxWaylandPlatformInput::keybordKey(wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (!m_keyboard.state) {
        return;
    }

    // Fetch the keysym of the pressed key
    // The +8 is an historical artifact
    const xkb_keycode_t keycode = key + 8;
    const auto keysym = xkb_state_key_get_one_sym(m_keyboard.state, keycode);

    // Lookup the KDGui key enum
    auto skey = xkb::keysymToKey(keysym);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        // Get the unicode characters
        xkb_state_key_get_utf8(m_keyboard.state, keycode, m_keyboard.repeat.text, sizeof(m_keyboard.repeat.text));

        m_keyboard.repeat.code = keycode;
        m_keyboard.repeat.key = skey;
        keyboardSendKeyPress(time, false);

        m_keyboard.repeat.time = time;
        m_keyboard.repeat.timer.interval = std::chrono::milliseconds(m_keyboard.repeat.delay);
        m_keyboard.repeat.timer.running = true;
        m_keyboard.repeat.elapsedTimer.start();
    } else {
        // Get the modifier state
        const auto modifiers = xkb::modifierState(m_keyboard.state);

        m_keyboard.focus->handleKeyRelease(time, keycode, skey, modifiers);
        m_keyboard.repeat.timer.running = false;
    }
}

void LinuxWaylandPlatformInput::keyboardModifiers(wl_keyboard *keyboard, uint32_t serial, uint32_t depressed,
                                                  uint32_t latched, uint32_t locked, uint32_t group)
{
    if (m_keyboard.state) {
        xkb_state_update_mask(m_keyboard.state, depressed, latched, locked, 0, 0, group);
    }
}

void LinuxWaylandPlatformInput::keyboardRepeatInfo(wl_keyboard *keyboard, int32_t rate, int32_t delay)
{
    m_keyboard.repeat.delay = delay;
    m_keyboard.repeat.rate = 1000 / rate;
}

void LinuxWaylandPlatformInput::keyboardRepeatKey()
{
    m_keyboard.repeat.timer.interval = std::chrono::milliseconds(m_keyboard.repeat.rate);

    m_keyboard.repeat.time += m_keyboard.repeat.elapsedTimer.msecElapsed();
    m_keyboard.repeat.elapsedTimer.restart();

    keyboardSendKeyPress(m_keyboard.repeat.time, true);
}

void LinuxWaylandPlatformInput::keyboardSendKeyPress(uint32_t time, bool isRepeat)
{
    // Get the modifier state
    const auto modifiers = xkb::modifierState(m_keyboard.state);

    if (isRepeat) {
        m_keyboard.focus->handleKeyRelease(time, m_keyboard.repeat.code, m_keyboard.repeat.key, modifiers);
    }

    // Generate a key press event
    m_keyboard.focus->handleKeyPress(time, m_keyboard.repeat.code, m_keyboard.repeat.key, modifiers);

    if (strlen(m_keyboard.repeat.text) > 0) {
        m_keyboard.focus->handleTextInput(m_keyboard.repeat.text);
    }
}

void LinuxWaylandPlatformInput::touchDown(wl_touch *touch, uint32_t serial, uint32_t time,
                                          wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    // We don't support touch yet, so just use the first touch point as mouse input
    if (m_touch.points.size() > 0) {
        return;
    }

    m_touch.focus = LinuxWaylandPlatformWindow::fromSurface(surface);
    m_touch.points.push_back(Touch::Point{ id, Position(wl_fixed_to_double(x), wl_fixed_to_double(y)) });
    m_touch.time = time;

    m_touch.focus->handleMouseMove(time, 1, m_touch.points[0].pos.x, m_touch.points[0].pos.y);
    m_touch.focus->handleMousePress(time, 1, m_touch.points[0].pos.x, m_touch.points[0].pos.y);
}

void LinuxWaylandPlatformInput::touchUp(wl_touch *touch, uint32_t serial, uint32_t time, int32_t id)
{
    if (id != m_touch.points[0].id) {
        return;
    }

    m_touch.focus->handleMouseRelease(time, 1, m_touch.points[0].pos.x, m_touch.points[0].pos.y);
    m_touch.points.clear();
}

void LinuxWaylandPlatformInput::touchMotion(wl_touch *touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y)
{
    if (id != m_touch.points[0].id) {
        return;
    }

    m_touch.focus->handleMouseMove(time, 1, m_touch.points[0].pos.x, m_touch.points[0].pos.y);
    m_touch.time = time;
}

void LinuxWaylandPlatformInput::touchFrame(wl_touch *touch)
{
}

void LinuxWaylandPlatformInput::touchCancel(wl_touch *touch)
{
    if (m_touch.points.size() > 0) {
        m_touch.focus->handleMouseRelease(m_touch.time, 1, m_touch.points[0].pos.x, m_touch.points[0].pos.y);
        m_touch.points.clear();
    }
}

void LinuxWaylandPlatformInput::touchShape(wl_touch *touch, int32_t id, wl_fixed_t minor, wl_fixed_t major)
{
}

void LinuxWaylandPlatformInput::touchOrientation(wl_touch *touch, int32_t id, wl_fixed_t orientation)
{
}
