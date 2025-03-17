/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_platform_integration.h"

#include <android_native_app_glue.h>
#include <KDUtils/file.h>
#include <KDUtils/logging.h>
#include <KDGui/window.h>
#include <KDFoundation/core_application.h>
#include <KDFoundation/platform/android/android_platform_event_loop.h>

#include <dlfcn.h>
#include <filesystem>

#include <android/log.h>

namespace KDFoundation {

android_app *AndroidPlatformIntegration::s_androidApp = nullptr;

AndroidPlatformIntegration::AndroidPlatformIntegration() = default;

AndroidPlatformIntegration::~AndroidPlatformIntegration() = default;

AbstractPlatformEventLoop *AndroidPlatformIntegration::createPlatformEventLoopImpl()
{
    return new AndroidPlatformEventLoop(this);
}

} // namespace KDFoundation

namespace {
std::string getAppName(struct android_app *app)
{
    auto *activity = app->activity;
    auto javaVM = activity->vm;
    JNIEnv *env = nullptr;

    // Attach the thread to the JVM
    if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        SPDLOG_WARN("Failed to attach thread to JVM");
        return {};
    }

    auto activityClass = env->GetObjectClass(activity->clazz);
    auto getPackageName = env->GetMethodID(activityClass, "getPackageName", "()Ljava/lang/String;");
    auto packageName = env->CallObjectMethod(activity->clazz, getPackageName);
    auto appName = env->GetStringUTFChars((jstring)packageName, nullptr);

    auto nameString = std::string(appName);
    env->ReleaseStringUTFChars((jstring)packageName, appName);

    // Detach the thread from the JVM
    javaVM->DetachCurrentThread();

    return nameString;
}

} // namespace

// Process the next main command.
static bool startMain = false;
static void handle_cmd(android_app *app, int32_t cmd)
{
    switch (cmd) {
    case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        startMain = true;
        break;
    case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        startMain = false;
        break;
    default:
        __android_log_print(ANDROID_LOG_INFO, "KDGuiAndroid",
                            "event not handled: %d", cmd);
    }
}

// Android app entry point
void android_main(struct android_app *app)
{
    using MainType = int (*)(int, const char **);
    // find "main" function
    const auto main = reinterpret_cast<MainType>(dlsym(RTLD_DEFAULT, "main"));
    // Set the callback to process system events
    app->onAppCmd = handle_cmd;
    KDFoundation::AndroidPlatformIntegration::s_androidApp = app;

    android_poll_source *source;
    do {
        if (ALooper_pollAll(0, nullptr, nullptr, (void **)&source) >= 0) {
            if (source != nullptr)
                source->process(app, source);
        }
    } while (app->destroyRequested == 0 && !startMain);

    static const auto appName = getAppName(app);
    auto appStr = appName.c_str();

    if (startMain && main)
        exit(main(1, &appStr));
    else
        exit(-1);
}
