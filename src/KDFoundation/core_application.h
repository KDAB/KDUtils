/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/object.h>
#include <KDFoundation/event_loop.h>
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

class KDFOUNDATION_API CoreApplication : public Object
{
public:
    KDBindings::Property<std::string> applicationName{};
    KDBindings::Property<std::string> organizationName{};

    CoreApplication(std::unique_ptr<AbstractPlatformIntegration> &&platformIntegration = {}, std::unique_ptr<KDFoundation::AbstractPlatformEventLoop> &&platformEventLoop = {});
    ~CoreApplication() override;

    static inline CoreApplication *instance() { return ms_application; }

    const EventLoop *eventLoop() const { return &m_eventLoop; }
    EventLoop *eventLoop() { return &m_eventLoop; }

    const AbstractPlatformEventLoop *platformEventLoop() const { return m_eventLoop.platformEventLoop(); }
    AbstractPlatformEventLoop *platformEventLoop() { return m_eventLoop.platformEventLoop(); }

    Postman *postman() { return m_eventLoop.postman(); }

    void postEvent(EventReceiver *target, std::unique_ptr<Event> &&event);
    void removeAllEventsTargeting(EventReceiver &evReceiver) { m_eventLoop.removeAllEventsTargeting(evReceiver); }
    EventQueue::size_type eventQueueSize() const { return m_eventLoop.eventQueueSize(); }

    void sendEvent(EventReceiver *target, Event *event);

    void processEvents(int timeout = 0);

    int exec();
    void quit();

    // Return a directory, at one of the standard directory locations
    KDUtils::Dir standardDir(StandardDir type) const;

    std::shared_ptr<KDBindings::ConnectionEvaluator> connectionEvaluator();
    AbstractPlatformIntegration *platformIntegration();

    static CoreApplication *ms_application;
    std::shared_ptr<spdlog::logger> m_defaultLogger;

private:
    void event(EventReceiver *target, Event *event) override;

    std::unique_ptr<AbstractPlatformIntegration> m_platformIntegration;
    EventLoop m_eventLoop;
    std::shared_ptr<spdlog::logger> m_logger;
};

} // namespace KDFoundation
