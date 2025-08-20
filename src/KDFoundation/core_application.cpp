/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

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

CoreApplication *CoreApplication::ms_application = nullptr;

namespace {
std::unique_ptr<AbstractPlatformIntegration> createPlatformIntegration()
{
// Create a platform integration
#if defined(PLATFORM_LINUX)
    return std::make_unique<LinuxPlatformIntegration>();
#elif defined(PLATFORM_WIN32)
    return std::make_unique<Win32PlatformIntegration>();
#elif defined(PLATFORM_MACOS)
    return std::make_unique<MacOSPlatformIntegration>();
#else
    static_assert(false, "No valid platform integration could be found.");
#endif
}
} // namespace

CoreApplication::CoreApplication(std::unique_ptr<AbstractPlatformIntegration> &&platformIntegration, std::unique_ptr<KDFoundation::AbstractPlatformEventLoop> &&platformEventLoop)
    : m_defaultLogger{ KDUtils::Logger::logger("default_log", spdlog::level::info) }
    , m_platformIntegration{ platformIntegration ? std::move(platformIntegration) : createPlatformIntegration() }
    , m_logger{ KDUtils::Logger::logger("core_application") }
    , m_eventLoop{ platformEventLoop ? std::move(platformEventLoop) : m_platformIntegration->createPlatformEventLoop() }
{
    assert(ms_application == nullptr);
    ms_application = this;

    spdlog::set_default_logger(m_defaultLogger);

    // Helps with debugging setup on remote hosts
    if (const char *display = std::getenv("DISPLAY")) // NOLINT(concurrency-mt-unsafe)
        SPDLOG_LOGGER_INFO(m_logger, "DISPLAY={}", display);

    m_platformIntegration->init();
}

CoreApplication::~CoreApplication()
{
    // Process pending events in case CoreApplication::quit() was not called before
    // and immediately return after processing events (timeout = 0).
    processEvents(0);

    // Destroy the platform integration before removing the app instance
    m_platformIntegration.reset();
    ms_application = nullptr;
}

std::shared_ptr<KDBindings::ConnectionEvaluator> CoreApplication::connectionEvaluator()
{
    return m_eventLoop.connectionEvaluator();
}

void CoreApplication::postEvent(EventReceiver *target, std::unique_ptr<Event> &&event)
{
    m_eventLoop.postEvent(target, std::forward<std::unique_ptr<Event>>(event));
}

void CoreApplication::sendEvent(EventReceiver *target, Event *event)
{
    m_eventLoop.sendEvent(target, event);
}

void CoreApplication::processEvents(int timeout)
{
    m_eventLoop.processEvents(timeout);
}

int CoreApplication::exec()
{
    return m_eventLoop.exec();
}

void CoreApplication::quit()
{
    postEvent(this, std::make_unique<QuitEvent>());
}

AbstractPlatformIntegration *CoreApplication::platformIntegration()
{
    return m_platformIntegration.get();
}

void CoreApplication::event(EventReceiver *target, Event *event)
{
    if (event->type() == Event::Type::Quit) {
        m_eventLoop.quit();
        event->setAccepted(true);
    }

    Object::event(target, event);
}

KDUtils::Dir CoreApplication::standardDir(StandardDir type) const
{
    return m_platformIntegration->standardDir(*this, type);
}
