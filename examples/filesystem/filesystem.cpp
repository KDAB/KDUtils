/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <KDGui/gui_application.h>
#include <KDGui/window.h>

namespace {
void doTests()
{
    auto app = KDGui::GuiApplication::instance();

    SPDLOG_INFO("******** Beginning \"{}\" Application by {} ********",
                app->applicationName(), app->organizationName());

    // Test system directories
    SPDLOG_INFO("Application Path:           \"{}\"", app->standardDir(KDFoundation::StandardDir::Application).path());
    SPDLOG_INFO("AppData Path:               \"{}\"", app->standardDir(KDFoundation::StandardDir::ApplicationData).path());
    SPDLOG_INFO("AppData Local Path:         \"{}\"", app->standardDir(KDFoundation::StandardDir::ApplicationDataLocal).path());
    SPDLOG_INFO("Assets Path:                \"{}\"", app->standardDir(KDFoundation::StandardDir::Assets).path());

#ifdef PLATFORM_ANDROID
    if (app->standardDir(KDFoundation::StandardDir::Assets).type() != KDUtils::StorageType::Asset)
        SPDLOG_CRITICAL("Assets Path is not of Asset type. Path: \"{}\"", app->standardDir(KDFoundation::StandardDir::Assets).path());
#endif

    // Try to find asset file
    auto textAssetDir = app->standardDir(KDFoundation::StandardDir::Assets).relativeDir("text");
    if (!textAssetDir.exists())
        SPDLOG_CRITICAL("Asset directory does not exist. Path: \"{}\"", textAssetDir.path());

    auto textAssetFile = textAssetDir.file("asset_text.txt");
    if (textAssetDir.exists())
        SPDLOG_INFO("Found asset file:           \"{}\"", textAssetFile.path());
    else
        SPDLOG_CRITICAL("Asset file does not exist. Path: \"{}\"", textAssetFile.path());

    // Read contents of asset file
    textAssetFile.open(std::ios_base::in);
    if (!textAssetFile.isOpen()) {
        SPDLOG_CRITICAL("Failed to open asset file for reading. Path: \"{}\"",
                        textAssetFile.path());
    } else {
        auto assetText = textAssetFile.readAll();
        SPDLOG_INFO("Asset file contents:        \"{}\"", assetText.toStdString());

        textAssetFile.close();
        if (textAssetFile.isOpen()) {
            SPDLOG_CRITICAL("Asset file is still open after closing. Path: \"{}\"",
                            textAssetFile.path());
        }
    }

    // Try creating a file in internal appdata
    auto internalTextOut = std::string("This is a test file created in internal appdata.");
    auto appDataDir = app->standardDir(KDFoundation::StandardDir::ApplicationDataLocal);
    appDataDir.mkdir({ true });
    auto testFileInternal = appDataDir.file("test_file_internal.txt");
    if (testFileInternal.exists())
        SPDLOG_CRITICAL("File already exists. Path: \"{}\"", testFileInternal.path());

    testFileInternal.open(std::ios_base::out);
    if (!testFileInternal.isOpen()) {
        SPDLOG_CRITICAL("Failed to open file for writing. Path: \"{}\"",
                        testFileInternal.path());
    } else {
        SPDLOG_INFO("Opened for writing:         \"{}\"", testFileInternal.path());
        testFileInternal.write(
                KDUtils::ByteArray(internalTextOut.c_str(), internalTextOut.length()));

        testFileInternal.close();
        if (testFileInternal.isOpen()) {
            SPDLOG_CRITICAL("File is still open after closing. Path: \"{}\"",
                            testFileInternal.path());
        }
    }

    // Read that file back
    testFileInternal.open(std::ios_base::in);
    if (!testFileInternal.isOpen()) {
        SPDLOG_CRITICAL("Failed to open file for reading. Path: \"{}\"",
                        testFileInternal.path());
    } else {
        SPDLOG_INFO("Opened for reading:         \"{}\"", testFileInternal.path());
        auto internalTextIn = testFileInternal.readAll().toStdString();
        SPDLOG_INFO("Internal file contents:     \"{}\"", internalTextIn);
        if (internalTextIn != internalTextOut)
            SPDLOG_CRITICAL("Text read does not match text written");

        testFileInternal.close();
        if (testFileInternal.isOpen()) {
            SPDLOG_CRITICAL("File is still open after closing. Path: \"{}\"",
                            testFileInternal.path());
        }
    }

    // Try creating a file in external appdata
    auto externalTextOut = std::string("This is a test file created in external appdata.");
    auto externalAppDataDir = app->standardDir(KDFoundation::StandardDir::ApplicationData).relativeDir("text/test");
    externalAppDataDir.mkdir({ true });
    if (!externalAppDataDir.exists())
        SPDLOG_CRITICAL("Failed to create external appdata directory. Path: \"{}\"", externalAppDataDir.path());

    auto testFileExternal = externalAppDataDir.file("test_file_external.txt");
    if (testFileExternal.exists())
        SPDLOG_CRITICAL("File already exists. Path: \"{}\"", testFileExternal.path());

    testFileExternal.open(std::ios_base::out);
    if (!testFileExternal.isOpen()) {
        SPDLOG_CRITICAL("Failed to open file for writing. Path: \"{}\"",
                        testFileExternal.path());
    } else {
        SPDLOG_INFO("Opened for writing:         \"{}\"", testFileExternal.path());
        testFileExternal.write(
                KDUtils::ByteArray(externalTextOut.c_str(), externalTextOut.length()));

        testFileExternal.close();
        if (testFileExternal.isOpen()) {
            SPDLOG_CRITICAL("File is still open after closing. Path: \"{}\"",
                            testFileExternal.path());
        }
    }

    // Read that file back
    testFileExternal.open(std::ios_base::in);
    if (!testFileExternal.isOpen()) {
        SPDLOG_CRITICAL("Failed to open file for reading. Path: \"{}\"",
                        testFileExternal.path());
    } else {
        SPDLOG_INFO("Opened for reading:         \"{}\"", testFileExternal.path());
        auto externalTextIn = testFileExternal.readAll().toStdString();
        SPDLOG_INFO("External file contents:     \"{}\"", externalTextIn);
        if (externalTextIn != externalTextOut)
            SPDLOG_CRITICAL("Text read does not match text written");

        testFileExternal.close();
        if (testFileExternal.isOpen()) {
            SPDLOG_CRITICAL("File is still open after closing. Path: \"{}\"",
                            testFileExternal.path());
        }
    }

    // Cleanup
    if (!testFileInternal.remove())
        SPDLOG_CRITICAL("Failed to remove internal test file. Path: \"{}\"", testFileInternal.path());
    else
        SPDLOG_INFO("Removed file:               \"{}\"", testFileInternal.path());

    if (!testFileExternal.remove())
        SPDLOG_CRITICAL("Failed to remove external test file. Path: \"{}\"", testFileExternal.path());
    else
        SPDLOG_INFO("Removed file:               \"{}\"", testFileExternal.path());

    if (!externalAppDataDir.rmdir())
        SPDLOG_CRITICAL("Failed to remove external appdata directory. Path: \"{}\"", externalAppDataDir.path());
    else
        SPDLOG_INFO("Removed directory:          \"{}\"", externalAppDataDir.path());

    if (!externalAppDataDir.parent().rmdir())
        SPDLOG_CRITICAL("Failed to remove external appdata parent directory. Path: \"{}\"", externalAppDataDir.parent().path());
    else
        SPDLOG_INFO("Removed directory:          \"{}\"", externalAppDataDir.parent().path());

    SPDLOG_INFO("******** \"{}\" Complete ********", app->applicationName());
    app->quit();
}
} // namespace

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
};

int main() // NOLINT(bugprone-exception-escape)
{
    KDGui::GuiApplication app;
    app.applicationName = "KDUtils Filesystem Tests";
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
