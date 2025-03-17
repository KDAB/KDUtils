/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Miłosz Kosobucki <milosz.kosobucki@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDGui/gui_application.h>
#include <KDGui/window.h>
#include <KDGui/gui_events.h>

#include <tuple>

class ExampleWindow : public KDGui::Window
{
    void mouseMoveEvent(KDGui::MouseMoveEvent *ev) override
    {
        SPDLOG_INFO("{}() buttons = {} at pos = ({}, {})",
                    __FUNCTION__,
                    ev->buttons().toInt(),
                    ev->xPos(),
                    ev->yPos());
    }

    void mousePressEvent(KDGui::MousePressEvent *ev) override
    {
        SPDLOG_INFO("{}() buttons = {} at pos = ({}, {})",
                    __FUNCTION__,
                    ev->buttons().toInt(),
                    ev->xPos(),
                    ev->yPos());
    }

    void mouseReleaseEvent(KDGui::MouseReleaseEvent *ev) override
    {
        SPDLOG_INFO("{}() buttons = {} at pos = ({}, {})",
                    __FUNCTION__,
                    ev->buttons().toInt(),
                    ev->xPos(),
                    ev->yPos());
    }

    void mouseWheelEvent(KDGui::MouseWheelEvent *ev) override
    {
        SPDLOG_INFO("{}() xDelta = {} yDelta = {}",
                    __FUNCTION__,
                    ev->xDelta(),
                    ev->yDelta());
    }

    void keyPressEvent(KDGui::KeyPressEvent *ev) override
    {
        SPDLOG_INFO("{}() key = {}", __FUNCTION__, static_cast<int>(ev->key()));
    }

    void keyReleaseEvent(KDGui::KeyReleaseEvent *ev) override
    {
        SPDLOG_INFO("{}() key = {}", __FUNCTION__, static_cast<int>(ev->key()));
    }
};

int main() // NOLINT(bugprone-exception-escape)
{
    KDGui::GuiApplication app;
    app.applicationName = "KDGui window example";

    ExampleWindow w;
    w.title = app.applicationName();
    w.visible = true;

    std::ignore = w.visible.valueChanged().connect([&app](bool visible) {
        if (!visible) {
            app.quit();
        }
    });

    app.exec();
}
