/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/event.h>
#include <KDGui/kdgui_keys.h>
#include <KDUtils/flags.h>

#include <string>

namespace KDGui {

enum MouseButton {
    NoButton = 0x00000000,
    LeftButton = 0x00000001,
    MiddleButton = 0x00000002,
    RightButton = 0x00000004
};
using MouseButtons = KDUtils::Flags<MouseButton>;

class MousePressEvent : public KDFoundation::Event
{
public:
    explicit MousePressEvent(uint32_t timestamp, MouseButton button,
                             MouseButtons buttons, int16_t xPos, int16_t yPos)
        : KDFoundation::Event(KDFoundation::Event::Type::MousePress)
        , m_timestamp{ timestamp }
        , m_button{ button }
        , m_buttons{ buttons }
        , m_xPos{ xPos }
        , m_yPos{ yPos }
    {
    }

    uint32_t timestamp() const { return m_timestamp; }
    /// @brief Button that was pressed
    MouseButton button() const { return m_button; }
    /// @brief Flags with all the buttons that are currently pressed
    MouseButtons buttons() const { return m_buttons; }
    int16_t xPos() const { return m_xPos; }
    int16_t yPos() const { return m_yPos; }

private:
    uint32_t m_timestamp;
    MouseButton m_button;
    MouseButtons m_buttons;
    int16_t m_xPos;
    int16_t m_yPos;
};

class MouseReleaseEvent : public KDFoundation::Event
{
public:
    explicit MouseReleaseEvent(uint32_t timestamp, MouseButton button,
                               MouseButtons buttons, int16_t xPos, int16_t yPos)
        : KDFoundation::Event(KDFoundation::Event::Type::MouseRelease)
        , m_timestamp{ timestamp }
        , m_button{ button }
        , m_buttons{ buttons }
        , m_xPos{ xPos }
        , m_yPos{ yPos }
    {
    }

    uint32_t timestamp() const { return m_timestamp; }
    /// @brief Button that was released
    MouseButton button() const { return m_button; }
    /// @brief Flags with all the buttons that are currently pressed
    ///
    /// The button for which this release was triggered will be set to false here.
    MouseButtons buttons() const { return m_buttons; }
    int16_t xPos() const { return m_xPos; }
    int16_t yPos() const { return m_yPos; }

private:
    uint32_t m_timestamp;
    MouseButton m_button;
    MouseButtons m_buttons;
    int16_t m_xPos;
    int16_t m_yPos;
};

class MouseMoveEvent : public KDFoundation::Event
{
public:
    explicit MouseMoveEvent(uint32_t timestamp, MouseButtons buttons,
                            int64_t xPos, int64_t yPos)
        : KDFoundation::Event(KDFoundation::Event::Type::MouseMove)
        , m_timestamp{ timestamp }
        , m_buttons{ buttons }
        , m_xPos{ xPos }
        , m_yPos{ yPos }
    {
    }

    uint32_t timestamp() const { return m_timestamp; }
    /// @brief Flags with all the buttons that are currently pressed
    MouseButtons buttons() const { return m_buttons; }
    int64_t xPos() const { return m_xPos; }
    int64_t yPos() const { return m_yPos; }

private:
    uint32_t m_timestamp;
    MouseButtons m_buttons;
    int64_t m_xPos;
    int64_t m_yPos;
};

class MouseWheelEvent : public KDFoundation::Event
{
public:
    explicit MouseWheelEvent(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
        : KDFoundation::Event(KDFoundation::Event::Type::MouseWheel)
        , m_timestamp{ timestamp }
        , m_xDelta{ xDelta }
        , m_yDelta{ yDelta }
    {
    }

    uint32_t timestamp() const { return m_timestamp; }
    int32_t xDelta() const { return m_xDelta; }
    int32_t yDelta() const { return m_yDelta; }

private:
    uint32_t m_timestamp;
    int32_t m_xDelta;
    int32_t m_yDelta;
};

class KeyEvent : public KDFoundation::Event
{
public:
    uint32_t timestamp() const { return m_timestamp; }
    uint8_t nativeKeycode() const { return m_nativeKeycode; }
    Key key() const { return m_key; }
    KeyboardModifiers modifiers() const { return m_modifiers; }

protected:
    explicit KeyEvent(KDFoundation::Event::Type type, uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
        : KDFoundation::Event(type)
        , m_timestamp{ timestamp }
        , m_nativeKeycode{ nativeKeycode }
        , m_key{ key }
        , m_modifiers{ modifiers }
    {
    }

private:
    uint32_t m_timestamp;
    uint8_t m_nativeKeycode;
    Key m_key{ Key_Unknown };
    KeyboardModifiers m_modifiers{ Mod_NoModifiers };
};

class KeyPressEvent : public KeyEvent
{
public:
    explicit KeyPressEvent(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
        : KeyEvent(KDFoundation::Event::Type::KeyPress, timestamp, nativeKeycode, key, modifiers)
    {
    }
};

class KeyReleaseEvent : public KeyEvent
{
public:
    explicit KeyReleaseEvent(uint32_t timestamp, uint8_t nativeKeycode, Key key, KeyboardModifiers modifiers)
        : KeyEvent(KDFoundation::Event::Type::KeyRelease, timestamp, nativeKeycode, key, modifiers)
    {
    }
};

class TextInputEvent : public KDFoundation::Event
{
public:
    explicit TextInputEvent(const std::string &chars)
        : KDFoundation::Event(KDFoundation::Event::Type::TextInput)
        , m_text{ chars }
    {
    }

    const std::string &text() const { return m_text; }

private:
    std::string m_text;
};

} // namespace KDGui

OPERATORS_FOR_FLAGS(KDGui::MouseButtons)
