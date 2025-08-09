/*
 *  This file is part of KDUtils.
 *
 *  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 *  Author: Paul Lemire <paul.lemire@kdab.com>
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Contact KDAB at <info@kdab.com> for commercial licensing options.
 */

#include <KDFoundation/event_loop.h>
#include <KDFoundation/core_application.h>
#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include "postman.h"

#if defined(PLATFORM_LINUX)
#include <KDFoundation/platform/linux/linux_platform_integration.h>
#elif defined(PLATFORM_WIN32)
#include <KDFoundation/platform/win32/win32_platform_integration.h>
#elif defined(PLATFORM_MACOS)
#include <KDFoundation/platform/macos/macos_platform_integration.h>
#endif

#include <cassert>

using namespace KDFoundation;

namespace {
thread_local EventLoop *s_eventLoopInstance = nullptr;
}

EventLoop *EventLoop::instance()
{
    return s_eventLoopInstance;
}

namespace {
std::unique_ptr<AbstractPlatformEventLoop> createPlatformEventLoop()
{
    // Ask the platform integration to create us a suitable event loop
    CoreApplication *app = CoreApplication::instance();
    assert(app && "Cannot use EventLoop without a CoreApplication.");
    AbstractPlatformIntegration *platformIntegration = app->platformIntegration();
    assert(platformIntegration);
    return platformIntegration->createPlatformEventLoop();
}
} // namespace

EventLoop::EventLoop(std::unique_ptr<AbstractPlatformEventLoop> platformEventLoop)
    : m_platformEventLoop(platformEventLoop ? std::move(platformEventLoop) : createPlatformEventLoop())
{
    // Create a default postman object
    m_postman = std::make_unique<Postman>();
    m_platformEventLoop->setPostman(m_postman.get());

    assert(s_eventLoopInstance == nullptr && "Cannot have more than one event loop per thread. Nested event loops are not supported.");
    s_eventLoopInstance = this;
}

EventLoop::~EventLoop()
{
    // Destroy the platform event loop before removing the event loop instance
    m_platformEventLoop.reset();
    s_eventLoopInstance = nullptr;
}

std::shared_ptr<KDBindings::ConnectionEvaluator> EventLoop::connectionEvaluator()
{
    return m_platformEventLoop->connectionEvaluator();
}

void EventLoop::postEvent(EventReceiver *target, std::unique_ptr<Event> &&event)
{
    assert(target != nullptr);
    assert(event->type() != Event::Type::Invalid);
    m_eventQueue.push(target, std::forward<std::unique_ptr<Event>>(event));
    m_platformEventLoop->wakeUp();
}

void EventLoop::sendEvent(EventReceiver *target, Event *event)
{
    m_postman->deliverEvent(target, event);
}

void EventLoop::processEvents(int timeout)
{
    // Deliver any events that have already been posted
    for (size_t eventIndex = 0, eventCount = m_eventQueue.size(); eventIndex < eventCount; ++eventIndex) {
        auto postedEvent = m_eventQueue.tryPop();
        if (!postedEvent)
            break;
        const auto target = postedEvent->target();
        const auto ev = postedEvent->wrappedEvent();

        m_postman->deliverEvent(target, ev);
    }

    // Poll/wait for new events
    if (!m_platformEventLoop)
        return;
    m_platformEventLoop->waitForEvents(timeout);
}

int EventLoop::exec()
{
    if (!m_platformEventLoop)
        return 1;

    while (!m_quitRequested)
        processEvents(-1);
    m_quitRequested = false;

    return 0;
}

void EventLoop::quit()
{
    // After setting the quit flag to true, we must wake up the event loop once more,
    // because processEvents() goes back into waitForEvents() after processing the
    // queued events. Without the wakeUp() call it would wait until some other event
    // woke it up again. Let's kick it ourselves.
    m_quitRequested = true;
    m_platformEventLoop->wakeUp();
}
