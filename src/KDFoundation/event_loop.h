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

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/object.h>
#include <KDFoundation/event_queue.h>
#include <KDFoundation/platform/abstract_platform_integration.h>

#include <KDUtils/logging.h>
#include <KDUtils/dir.h>

#include <kdbindings/property.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace KDFoundation {

class AbstractPlatformEventLoop;
class Postman;

class KDFOUNDATION_API EventLoop
{
public:
    EventLoop(std::unique_ptr<AbstractPlatformEventLoop> platformEventLoop = nullptr);
    ~EventLoop();

    static EventLoop *instance();

    const AbstractPlatformEventLoop *platformEventLoop() const { return m_platformEventLoop.get(); }
    AbstractPlatformEventLoop *platformEventLoop() { return m_platformEventLoop.get(); }

    Postman *postman() { return m_postman.get(); }

    void postEvent(EventReceiver *target, std::unique_ptr<Event> &&event);
    void removeAllEventsTargeting(EventReceiver &evReceiver) { m_eventQueue.removeAllEventsTargeting(evReceiver); }
    EventQueue::size_type eventQueueSize() const { return m_eventQueue.size(); }

    void sendEvent(EventReceiver *target, Event *event);

    void processEvents(int timeout = 0);

    int exec();
    void quit();

    std::shared_ptr<KDBindings::ConnectionEvaluator> connectionEvaluator();

private:
    EventQueue m_eventQueue;
    bool m_quitRequested = false;
    std::unique_ptr<AbstractPlatformEventLoop> m_platformEventLoop;
    std::unique_ptr<Postman> m_postman;
};

} // namespace KDFoundation
