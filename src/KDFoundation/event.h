/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDFoundation/event_receiver.h"
#include <KDFoundation/kdfoundation_global.h>

#include <cstdint>
#include <memory>

namespace KDFoundation {

class KDFOUNDATION_API Event
{
public:
    enum class Type : uint16_t {
        Invalid = 0,
        Timer = 1,
        PostedEvent = 2,
        Notifier = 3,
        Quit = 4,
        Resize = 5,
        MousePress = 6,
        MouseRelease = 7,
        MouseDoubleClick = 8,
        MouseMove = 9,
        MouseWheel = 10,
        KeyPress = 11,
        KeyRelease = 12,
        TextInput = 13,
        Update = 14,
        DeferredDelete = 15,

        UserType = 4096
    };

    Event() = delete;
    virtual ~Event();

    explicit Event(Type type)
        : m_type{ type }
        , m_systemEvent{ false }
        , m_accepted{ false }
        , m_postedToEventQueue{ false }
        , m_reserved{ 0 }
    {
    }

    // Is not copyable
    Event(Event const &other) = delete;
    Event &operator=(Event const &other) = delete;

    // Is movable
    Event(Event &&other) noexcept = default;
    Event &operator=(Event &&other) noexcept = default;

    Type type() const { return m_type; }

    bool isSystemEvent() const { return m_systemEvent; }
    bool isManualEvent() const { return !m_systemEvent; }

    bool isAccepted() const { return m_accepted; }
    void setAccepted(bool accepted) { m_accepted = accepted; }

protected:
    Type m_type;
    uint8_t m_systemEvent : 1;
    uint8_t m_accepted : 1;
    uint8_t m_postedToEventQueue : 1;
    uint8_t m_reserved : 5;
};

class Object;

class KDFOUNDATION_API PostedEvent : public Event
{
public:
    PostedEvent(EventReceiver *target, std::unique_ptr<Event> &&wrappedEvent)
        : Event(Event::Type::PostedEvent)
        , m_target{ target }
        , m_wrappedEvent{ std::move(wrappedEvent) }
    {
    }

    EventReceiver *target() const { return m_target; }
    Event *wrappedEvent() const { return m_wrappedEvent.get(); }

private:
    EventReceiver *m_target;
    std::unique_ptr<Event> m_wrappedEvent;
};

// TODO: Implement a timer :)
class KDFOUNDATION_API TimerEvent : public Event
{
public:
    TimerEvent()
        : Event(Event::Type::Timer)
    {
    }
};

class KDFOUNDATION_API NotifierEvent : public Event
{
public:
    NotifierEvent()
        : Event(Event::Type::Notifier)
    {
    }
};

class KDFOUNDATION_API QuitEvent : public Event
{
public:
    QuitEvent()
        : Event(Event::Type::Quit)
    {
    }
};

class KDFOUNDATION_API ResizeEvent : public Event
{
public:
    explicit ResizeEvent(uint32_t width, uint32_t height)
        : Event(Event::Type::Resize)
        , m_width{ width }
        , m_height{ height }
    {
    }

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

private:
    uint32_t m_width;
    uint32_t m_height;
};

class KDFOUNDATION_API UpdateEvent : public Event
{
public:
    UpdateEvent()
        : Event(Event::Type::Update)
    {
    }
};

class KDFOUNDATION_API DeferredDeleteEvent : public Event
{
public:
    DeferredDeleteEvent()
        : Event(Event::Type::DeferredDelete)
    {
    }
};

} // namespace KDFoundation
