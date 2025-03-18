/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDGui/gui_application.h>
#include <KDGui/window.h>
#include <KDGui/gui_events.h>

#include <tuple>

class TestingWindow : public KDGui::Window
{

protected:
    void event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev) override
    {
        // Handle events we care about
        if (target == this && ev->type() == KDFoundation::Event::Type::Update) {
            doTests();
        }

        KDGui::Window::event(target, ev);
    }

private:
    void doTests()
    {
        auto app = KDGui::GuiApplication::instance();

        SPDLOG_INFO("******** Beginning \"{}\" Application by {} ********",
                    app->applicationName(), app->organizationName());

        // Test system directories
        SPDLOG_INFO("Application Path: \"{}\"", app->applicationDir().path());
        SPDLOG_INFO("AppData Path:     \"{}\"", app->applicationDataDir().path());
        SPDLOG_INFO("Assets Path:      \"{}\"", app->assetsDir().path());

        SPDLOG_INFO("******** \"{}\" Complete ********", app->applicationName());
        app->quit();
    }
};

int main() // NOLINT(bugprone-exception-escape)
{
    KDGui::GuiApplication app;
    app.applicationName = "KDUtils Android Platform Tests";
    app.organizationName = "KDAB";

    TestingWindow w;
    w.title = app.applicationName();
    w.visible = true;

    std::ignore = w.visible.valueChanged().connect([&app](bool visible) {
        if (!visible) {
            app.quit();
        }
    });

    // Ensure that the app window will receive an event at the first opportunity that results
    // in running the tests
    app.postEvent(&w, std::make_unique<KDFoundation::UpdateEvent>());

    // Run the app
    app.exec();
}
