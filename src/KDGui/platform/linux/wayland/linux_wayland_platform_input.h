/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <xkbcommon/xkbcommon.h>

#include <KDUtils/elapsedtimer.h>
#include <KDFoundation/timer.h>
#include <KDGui/kdgui_keys.h>
#include <KDGui/position.h>

struct wl_array;
struct wl_seat;
struct wl_surface;
struct wl_pointer;
struct wl_keyboard;
struct wl_touch;
struct wl_cursor_theme;
struct zwp_relative_pointer_v1;
struct zwp_locked_pointer_v1;
using wl_fixed_t = int32_t;

namespace KDGui {

class LinuxWaylandPlatformIntegration;
class LinuxWaylandPlatformWindow;

class LinuxWaylandPlatformInput
{
public:
    static constexpr uint32_t supportedVersion = 7;
    static constexpr uint32_t relativePointerV1SupportedVersion = 1;
    static constexpr uint32_t pointerConstraintsV1SupportedVersion = 1;

    explicit LinuxWaylandPlatformInput(LinuxWaylandPlatformIntegration *integration, wl_seat *seat, uint32_t version, uint32_t id);
    ~LinuxWaylandPlatformInput();

    uint32_t seatId() const { return m_seatId; }
    wl_seat *seat() const { return m_seat; }

    void destroyRelativePointerV1();
    void destroyPointerConstraintsV1();

private:
    void capabilities(wl_seat *seat, uint32_t caps);
    void name(wl_seat *seat, const char *name);

    void initPointer();
    void destroyPointer();
    void initKeyboard();
    void destroyKeyboard();
    void initTouch();
    void destroyTouch();
    void setCursor();
    void pointerLock();
    void pointerUnlock();

    void pointerEnter(wl_pointer *pointer, uint32_t serial, wl_surface *surface, wl_fixed_t x, wl_fixed_t y);
    void pointerLeave(wl_pointer *pointer, uint32_t serial, wl_surface *surface);
    void pointerMotion(wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
    void pointerButton(wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void pointerAxis(wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
    void pointerFrame(wl_pointer *pointer);
    void pointerAxisSource(wl_pointer *pointer, uint32_t axis);
    void pointerAxisStop(wl_pointer *pointer, uint32_t time, uint32_t axis);
    void pointerAxisDiscrete(wl_pointer *pointer, uint32_t axis, int32_t steps);
    void pointerRelativeMotionV1(zwp_relative_pointer_v1 *pointer, uint32_t utimeHi, uint32_t utimeLow, wl_fixed_t dx, wl_fixed_t dy,
                                 wl_fixed_t dxUnaccel, wl_fixed_t dyUnaccel);

    void keyboardKeymap(wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    void keyboardEnter(wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    void keyboardLeave(wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    void keybordKey(wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void keyboardModifiers(wl_keyboard *keyboard, uint32_t serial, uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group);
    void keyboardRepeatInfo(wl_keyboard *keyboard, int32_t rate, int32_t delay);
    void keyboardRepeatKey();
    void keyboardSendKeyPress(uint32_t time, bool isRepeat);

    void touchDown(wl_touch *touch, uint32_t serial, uint32_t time, wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
    void touchUp(wl_touch *touch, uint32_t serial, uint32_t time, int32_t id);
    void touchMotion(wl_touch *touch, uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
    void touchFrame(wl_touch *touch);
    void touchCancel(wl_touch *touch);
    void touchShape(wl_touch *touch, int32_t id, wl_fixed_t minor, wl_fixed_t major);
    void touchOrientation(wl_touch *touch, int32_t id, wl_fixed_t orientation);

    LinuxWaylandPlatformIntegration *m_integration;
    wl_seat *m_seat;
    uint32_t m_version;
    uint32_t m_seatId;
    std::string m_name;

    struct Pointer {
        uint32_t lastSerial = 0;
        wl_pointer *pointer{ nullptr };
        zwp_relative_pointer_v1 *relativePointerV1{ nullptr };
        zwp_locked_pointer_v1 *lockedPointerV1{ nullptr };
        wl_surface *cursorSurface{ nullptr };
        LinuxWaylandPlatformWindow *focus{ nullptr };
        Position pos{ 0, 0 };
        struct AccumulatedPointerEvent {
            uint32_t time{ 0 };
            Position axis{ 0, 0 };
            Position pos{ 0, 0 };
            Position delta{ 0, 0 };
            LinuxWaylandPlatformWindow *focus{ nullptr };
            int focusChange : 1;
        } accumulatedEvent;
    } m_pointer;

    struct Keyboard {
        wl_keyboard *keyboard{ nullptr };
        LinuxWaylandPlatformWindow *focus{ nullptr };
        xkb_context *context{ nullptr };
        xkb_keymap *keymap{ nullptr };
        xkb_state *state{ nullptr };
        uint32_t serial{ 0 };

        struct {
            KDFoundation::Timer timer;
            int32_t rate;
            int32_t delay;
            xkb_keycode_t code;
            KDGui::Key key;
            char text[16];
            uint32_t time;
            KDUtils::ElapsedTimer elapsedTimer;
        } repeat;
    } m_keyboard;

    struct Touch {
        wl_touch *touch{ nullptr };
        LinuxWaylandPlatformWindow *focus{ nullptr };
        uint32_t time{ 0 };
        struct Point {
            int32_t id;
            Position pos;
        };
        std::vector<Point> points;
    } m_touch;
};

} // namespace KDGui
