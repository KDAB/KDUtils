/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "window.h"
#include "gui_application.h"
#include "gui_events.h"

#include <KDFoundation/platform/abstract_platform_integration.h>
#include <KDFoundation/event.h>

using namespace KDFoundation;
using namespace KDGui;

Window::Window()
    : Object()
{
    m_logger = spdlog::get("window");
    if (!m_logger) {
        m_logger = spdlog::stdout_color_mt("window");
        m_logger->set_level(spdlog::level::info);
    }

    visible.valueChanged().connect(&Window::onVisibleChanged, this);
    m_resizeConnectionIds = {
        width.valueChanged().connect(&Window::onSizeChanged, this),
        height.valueChanged().connect(&Window::onSizeChanged, this)
    };
    cursorEnabled.valueChanged().connect(&Window::onCursorEnabledChanged, this);
    rawMouseInputEnabled.valueChanged().connect(&Window::onRawMouseInputEnabledChanged, this);
}

Window::~Window() = default;

Window::Window(Window &&other) noexcept = default;
Window &Window::operator=(Window &&other) noexcept = default;

void Window::createPlatformWindow()
{
    if (m_platformWindow)
        return;

    auto app = GuiApplication::instance();
    if (!app) {
        m_logger->warn("No application object exists. Cannot create a platform window");
        return;
    }

    auto platfromIntegration = app->guiPlatformIntegration();
    if (!platfromIntegration) {
        m_logger->warn("No platform integration exists. Cannot create a platform window");
        return;
    }

    m_platformWindow = platfromIntegration->createPlatformWindow(this);
}

void Window::create()
{
    if (!m_platformWindow)
        createPlatformWindow();

    // If the above failed (e.g. we don't have an app instance) then we should bail
    if (!m_platformWindow)
        return;

    m_platformWindow->create();
}

void Window::destroy()
{
    if (!m_platformWindow)
        createPlatformWindow();

    // If the above failed (e.g. we don't have an app instance) then we should bail
    if (!m_platformWindow)
        return;

    m_platformWindow->destroy();
}

void Window::registerEventReceiver(Object *receiver)
{
    auto it = std::find(m_eventReceivers.begin(), m_eventReceivers.end(), receiver);
    if (it == m_eventReceivers.end())
        m_eventReceivers.push_back(receiver);
}

void Window::unregisterEventReceiver(Object *receiver)
{
    auto it = std::find(m_eventReceivers.begin(), m_eventReceivers.end(), receiver);
    if (it != m_eventReceivers.end())
        m_eventReceivers.erase(it);
}

void Window::onVisibleChanged(const bool &visible)
{
    SPDLOG_LOGGER_INFO(m_logger, "{}()", __FUNCTION__);
    create();
    if (visible) {
        m_platformWindow->map();
    } else {
        m_platformWindow->unmap();
    }
}

void Window::onSizeChanged()
{
    SPDLOG_LOGGER_INFO(m_logger, "{}()", __FUNCTION__);
    if (m_platformWindow && m_platformWindow->isCreated())
        m_platformWindow->setSize(width.get(), height.get());
}

void Window::onCursorEnabledChanged(const bool &cursorEnabled)
{
    SPDLOG_LOGGER_INFO(m_logger, "{}()", __FUNCTION__);
    if (m_platformWindow && m_platformWindow->isCreated()) {
        if (cursorEnabled) {
            m_platformWindow->enableCursor();
        } else {
            m_platformWindow->disableCursor();
        }
    }
}

void Window::onRawMouseInputEnabledChanged(const bool &rawMouseInputEnabled)
{
    SPDLOG_LOGGER_INFO(m_logger, "{}()", __FUNCTION__);
    if (m_platformWindow && m_platformWindow->isCreated()) {
        if (rawMouseInputEnabled) {
            m_platformWindow->enableRawMouseInput();
        } else {
            m_platformWindow->disableRawMouseInput();
        }
    }
}

void Window::event(EventReceiver *target, Event *ev)
{
    SPDLOG_LOGGER_TRACE(m_logger, "{}()", __FUNCTION__);

    // If we have an overlay, give it first dibs on events
    for (Object *eventReceiver : m_eventReceivers) {
        eventReceiver->event(target, ev);
        if (ev->isAccepted())
            return;
    }

    switch (ev->type()) {
    case Event::Type::Resize: {
        resizeEvent(static_cast<ResizeEvent *>(ev));
        break;
    }

    case Event::Type::MousePress: {
        mousePressEvent(static_cast<MousePressEvent *>(ev));
        break;
    }

    case Event::Type::MouseRelease: {
        mouseReleaseEvent(static_cast<MouseReleaseEvent *>(ev));
        break;
    }

    case Event::Type::MouseMove: {
        mouseMoveEvent(static_cast<MouseMoveEvent *>(ev));
        break;
    }

    case Event::Type::KeyPress: {
        keyPressEvent(static_cast<KeyPressEvent *>(ev));
        break;
    }

    case Event::Type::KeyRelease: {
        keyReleaseEvent(static_cast<KeyReleaseEvent *>(ev));
        break;
    }

    case Event::Type::MouseWheel: {
        mouseWheelEvent(static_cast<MouseWheelEvent *>(ev));
        break;
    }

    default: {
        break;
    }
    }

    Object::event(target, ev);
}

void Window::resizeEvent(ResizeEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger,
                        "{}() size = {} x {}",
                        __FUNCTION__,
                        ev->width(),
                        ev->height());
    // Block internal connection to onSizeChanged() to avoid a loop
    {
        KDBindings::ConnectionBlocker widthBlocker{ m_resizeConnectionIds[0] };
        width = ev->width();
    }

    {
        KDBindings::ConnectionBlocker heightBlocker{ m_resizeConnectionIds[1] };
        height = ev->height();
    }

    ev->setAccepted(true);
}

void Window::mousePressEvent(MousePressEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger,
                        "{}() button = {} at pos = ({}, {})",
                        __FUNCTION__,
                        ev->button(),
                        ev->xPos(),
                        ev->yPos());
}

void Window::mouseReleaseEvent(MouseReleaseEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger,
                        "{}() button = {} at pos = ({}, {})",
                        __FUNCTION__,
                        ev->button(),
                        ev->xPos(),
                        ev->yPos());
}

void Window::mouseMoveEvent(MouseMoveEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger,
                        "{}() button = {} at pos = ({}, {})",
                        __FUNCTION__,
                        ev->button(),
                        ev->xPos(),
                        ev->yPos());

    cursorPosition = Position(ev->xPos(), ev->yPos());
}

void Window::mouseWheelEvent(MouseWheelEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger,
                        "{}() xDelta = {} yDelta = {}",
                        __FUNCTION__,
                        ev->xDelta(),
                        ev->yDelta());
}

void Window::keyPressEvent(KeyPressEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger, "{}() key = {}", __FUNCTION__, ev->key());
}

void Window::keyReleaseEvent(KeyReleaseEvent *ev)
{
    SPDLOG_LOGGER_DEBUG(m_logger, "{}() key = {}", __FUNCTION__, ev->key());
}
