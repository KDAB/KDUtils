/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>
#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/logging.h>
#include <KDGui/kdgui_global.h>
#include <KDGui/position.h>

#include <kdbindings/property.h>

#include <memory>
#include <string>
#include <array>

namespace KDFoundation {
class ResizeEvent;
}

namespace KDGui {

class AbstractPlatformWindow;
class KeyPressEvent;
class KeyReleaseEvent;
class MouseMoveEvent;
class MousePressEvent;
class MouseReleaseEvent;
class MouseWheelEvent;

class KDGUI_API Window : public KDFoundation::Object
{
public:
    KDBindings::Property<std::string> title{ "KDGui" };
    KDBindings::Property<bool> visible{ false };
    KDBindings::Property<bool> cursorEnabled{ true };
    KDBindings::Property<bool> rawMouseInputEnabled{ false };
    KDBindings::Property<uint32_t> width{ 800 };
    KDBindings::Property<uint32_t> height{ 600 };
    KDBindings::Property<Position> cursorPosition{ Position(0, 0) };
    KDBindings::Property<float> scaleFactor{ 1.f };

    Window();
    ~Window();

    Window(Window &&other) noexcept;
    Window &operator=(Window &&other) noexcept;

    void create();
    void destroy();
    bool isCreated() const { return m_platformWindow != nullptr; }
    AbstractPlatformWindow *platformWindow() const { return m_platformWindow.get(); }

    void registerEventReceiver(KDFoundation::Object *receiver);
    void unregisterEventReceiver(KDFoundation::Object *receiver);

protected:
    void createPlatformWindow();

    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override;
    virtual void resizeEvent(KDFoundation::ResizeEvent *ev);
    virtual void mousePressEvent(MousePressEvent *ev);
    virtual void mouseReleaseEvent(MouseReleaseEvent *ev);
    virtual void mouseMoveEvent(MouseMoveEvent *ev);
    virtual void mouseWheelEvent(MouseWheelEvent *ev);
    virtual void keyPressEvent(KeyPressEvent *ev);
    virtual void keyReleaseEvent(KeyReleaseEvent *ev);

private:
    void onVisibleChanged(const bool &visible);
    void onSizeChanged();
    void onCursorEnabledChanged(const bool &cursorEnabled);
    void onRawMouseInputEnabledChanged(const bool &rawMouseInputEnabled);

    std::unique_ptr<AbstractPlatformWindow> m_platformWindow;
    std::array<KDBindings::ConnectionHandle, 2> m_resizeConnectionIds;
    std::shared_ptr<spdlog::logger> m_logger;
    std::vector<KDFoundation::Object *> m_eventReceivers;
};

} // namespace KDGui
