/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/object.h>
#include <KDFoundation/event_queue.h>
#include <KDFoundation/platform/abstract_platform_integration.h>

#include <KDUtils/logging.h>

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

    CoreApplication(std::unique_ptr<AbstractPlatformIntegration> &&platformIntegration = {});
    ~CoreApplication() override;

    static inline CoreApplication *instance() { return ms_application; }

    const AbstractPlatformEventLoop *eventLoop() const { return m_platformEventLoop.get(); }
    AbstractPlatformEventLoop *eventLoop() { return m_platformEventLoop.get(); }

    Postman *postman() { return m_postman.get(); }

    void postEvent(EventReceiver *target, std::unique_ptr<Event> &&event);
    EventQueue::size_type eventQueueSize() const { return m_eventQueue.size(); }

    void sendEvent(EventReceiver *target, Event *event);

    void processEvents(int timeout = 0);

    int exec();
    void quit();

    AbstractPlatformIntegration *platformIntegration();

    static CoreApplication *ms_application;
    static bool ms_loggingSetup;
    std::shared_ptr<spdlog::logger> m_logger;

private:
    void init();
    void event(EventReceiver *target, Event *event) override;

    EventQueue m_eventQueue;
    bool m_quitRequested{ false };
    std::unique_ptr<AbstractPlatformIntegration> m_platformIntegration;
    std::unique_ptr<AbstractPlatformEventLoop> m_platformEventLoop;
    std::unique_ptr<Postman> m_postman;
};

} // namespace KDFoundation
