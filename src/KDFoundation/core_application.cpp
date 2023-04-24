/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include <spdlog_setup/conf.h>

#include <cassert>

using namespace KDFoundation;

CoreApplication *CoreApplication::ms_application = nullptr;
bool CoreApplication::ms_loggingSetup = false;
static std::vector<CoreApplication::FileReader> s_fileReaders;

static std::optional<std::vector<uint8_t>> readLocalFile(const std::string &filename)
{
    auto logger = spdlog::get("application");
    if (!logger)
        logger = createLogger("application");
    SPDLOG_LOGGER_TRACE(logger, "Loading file: {}", filename);
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return {};

    auto res = std::make_optional<std::vector<uint8_t>>();
    const size_t fileSize = static_cast<size_t>(file.tellg());
    res->resize(fileSize);
    file.seekg(0);
    file.read(reinterpret_cast<char *>(res->data()), static_cast<std::streamsize>(fileSize));
    file.close();
    return res;
}

CoreApplication::CoreApplication(std::unique_ptr<AbstractPlatformIntegration> &&platformIntegration)
    : Object()
    , m_platformIntegration{ std::move(platformIntegration) }
{
    assert(ms_application == nullptr);
    ms_application = this;

    const char *logConfig = std::getenv("KDFOUNDATION_LOG_CONFIG");
    if (logConfig && !ms_loggingSetup) {
        try {
            spdlog_setup::from_file(logConfig);
            ms_loggingSetup = true;
        } catch (const spdlog_setup::setup_error &e) {
            spdlog::critical("Failed to setup logging: {}", e.what());
            exit(1);
        } catch (const std::exception &) {
            spdlog::critical("Failed to setup logging");
            exit(1);
        }
    }

    m_logger = spdlog::get("application");
    if (!m_logger) {
        m_logger = createLogger("application");
        m_logger->set_level(spdlog::level::info);
    }

    // Helps with debugging setup on remote hosts
    if (const char *display = std::getenv("DISPLAY"))
        SPDLOG_LOGGER_INFO(m_logger, "DISPLAY={}", display);

    s_fileReaders.emplace_back(readLocalFile);
    // Create a default postman object
    m_postman = std::make_unique<Postman>();

    // Create a platform integration
    if (!m_platformIntegration) {
#if defined(PLATFORM_LINUX)
        m_platformIntegration = std::make_unique<LinuxPlatformIntegration>();
#elif defined(PLATFORM_WIN32)
        m_platformIntegration = std::make_unique<Win32PlatformIntegration>();
#elif defined(PLATFORM_MACOS)
        m_platformIntegration = std::make_unique<MacOSPlatformIntegration>();
#endif
    }

    // Ask the platform integration to create us a suitable event loop
    assert(m_platformIntegration);
    m_platformEventLoop = m_platformIntegration->createPlatformEventLoop();
    m_platformIntegration->init();
    m_platformEventLoop->setPostman(m_postman.get());
}

CoreApplication::~CoreApplication()
{
    // Destroy the event loop and platform integration before removing the app instance
    m_platformEventLoop = std::unique_ptr<AbstractPlatformEventLoop>();
    m_platformIntegration = std::unique_ptr<AbstractPlatformIntegration>();
    ms_application = nullptr;
}

void CoreApplication::postEvent(Object *target, std::unique_ptr<Event> &&event)
{
    assert(target != nullptr);
    assert(event->type() != Event::Type::Invalid);
    m_eventQueue.push(target, std::forward<std::unique_ptr<Event>>(event));
    m_platformEventLoop->wakeUp();
}

void CoreApplication::sendEvent(Object *target, Event *event)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}()", __FUNCTION__);
    m_postman->deliverEvent(target, event);
}

void CoreApplication::processEvents(int timeout)
{
    // Deliver any events that have already been posted
    const auto eventCount = m_eventQueue.size();
    int eventIndex{ 0 };
    do {
        auto postedEvent = m_eventQueue.tryPop();
        if (!postedEvent)
            break;

        auto target = postedEvent->target();
        auto ev = postedEvent->wrappedEvent();
        m_postman->deliverEvent(target, ev);
    } while (++eventIndex < eventCount);

    // Poll/wait for new events
    if (!m_platformEventLoop)
        return;
    m_platformEventLoop->waitForEvents(timeout);
}

int CoreApplication::exec()
{
    if (!m_platformEventLoop)
        return 1;

    while (!m_quitRequested)
        processEvents(-1);
    m_quitRequested = false;

    return 0;
}

void CoreApplication::quit()
{
    postEvent(this, std::make_unique<QuitEvent>());
}

AbstractPlatformIntegration *CoreApplication::platformIntegration()
{
    return m_platformIntegration.get();
}

void CoreApplication::registerFileReader(const FileReader &fileReader)
{
    s_fileReaders.push_back(fileReader);
}

std::vector<uint8_t> CoreApplication::readFile(const std::string &filename)
{
    for (const auto &fr : s_fileReaders) {
        if (auto res = fr(filename)) {
            return std::move(*res);
        }
    }
    auto logger = spdlog::get("application");
    if (!logger)
        logger = createLogger("application");

    SPDLOG_LOGGER_ERROR(logger, "Can't load: {}", filename);

    throw std::runtime_error("failed to open file!");
}

void CoreApplication::event(EventReceiver *target, Event *event)
{
    if (event->type() == Event::Type::Quit) {
        // After setting the quit flag to true, we must wake up the event loop once more,
        // because processEvents() goes back into waitForEvents() after processing the
        // queued events. Without the wakeUp() call it would wait until some other event
        // woke it up again. Let's kick it ourselves.
        m_quitRequested = true;
        m_platformEventLoop->wakeUp();
        event->setAccepted(true);
    }

    Object::event(target, event);
}
