/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Miłosz Kosobucki <milosz.kosobucki@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDGui/gui_application.h>
#include <KDGui/window.h>
#include <KDGui/gui_events.h>

class ExampleWindow : public KDGui::Window
{
    void mouseMoveEvent(KDGui::MouseMoveEvent *ev) override
    {
        spdlog::info("{}() buttons = {} at pos = ({}, {})",
                     __FUNCTION__,
                     ev->buttons(),
                     ev->xPos(),
                     ev->yPos());
    }

    void mousePressEvent(KDGui::MousePressEvent *ev) override
    {
        spdlog::info("{}() buttons = {} at pos = ({}, {})",
                     __FUNCTION__,
                     ev->buttons(),
                     ev->xPos(),
                     ev->yPos());
    }

    void mouseReleaseEvent(KDGui::MouseReleaseEvent *ev) override
    {
        spdlog::info("{}() buttons = {} at pos = ({}, {})",
                     __FUNCTION__,
                     ev->buttons(),
                     ev->xPos(),
                     ev->yPos());
    }

    void mouseWheelEvent(KDGui::MouseWheelEvent *ev) override
    {
        spdlog::info("{}() xDelta = {} yDelta = {}",
                     __FUNCTION__,
                     ev->xDelta(),
                     ev->yDelta());
    }

    void keyPressEvent(KDGui::KeyPressEvent *ev) override
    {
        spdlog::info("{}() key = {}", __FUNCTION__, ev->key());
    }

    void keyReleaseEvent(KDGui::KeyReleaseEvent *ev) override
    {
        spdlog::info("{}() key = {}", __FUNCTION__, ev->key());
    }
};

int main()
{
    KDGui::GuiApplication app;

    ExampleWindow w;
    w.title = "KDGui window example";
    w.visible = true;

    w.visible.valueChanged().connect([&app](bool visible) {
        if (!visible) {
            app.quit();
        }
    });

    app.exec();
}
