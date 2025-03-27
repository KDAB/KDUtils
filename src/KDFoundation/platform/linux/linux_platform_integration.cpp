/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_integration.h>

#include <KDFoundation/core_application.h>

using namespace KDFoundation;

LinuxPlatformIntegration::LinuxPlatformIntegration()
{
}

LinuxPlatformIntegration::~LinuxPlatformIntegration()
{
}

LinuxPlatformEventLoop *LinuxPlatformIntegration::createPlatformEventLoopImpl()
{
    return new LinuxPlatformEventLoop();
}

KDUtils::Dir LinuxPlatformIntegration::standardDir(const CoreApplication &app, KDFoundation::StandardDir type) const
{
    switch (type) {
    case StandardDir::Application:
        return KDUtils::Dir::applicationDir();
    case StandardDir::ApplicationData:
    case StandardDir::ApplicationDataLocal:
        return KDUtils::Dir(linuxAppDataPath(app));
    case StandardDir::Assets:
        return KDUtils::Dir(KDUtils::Dir::applicationDir().parent().absoluteFilePath("assets"));
    default:
        SPDLOG_WARN("Unsupported standard directory requested");
        return {};
    }
}

std::string KDFoundation::LinuxPlatformIntegration::linuxAppDataPath(const CoreApplication &app)
{
    auto homePath = secure_getenv("HOME");
    auto appDataPath = std::string(homePath ? homePath : "") + "/.local/share";

    auto appName = app.applicationName();
    if (appName.empty()) {
        SPDLOG_CRITICAL("Application name is required to be set in order to generate an Application Data directory path");
        return {};
    }

    const auto orgName = app.organizationName();
    if (orgName.empty()) {
        SPDLOG_WARN("No Organization name - using only Application name for the directory");
    } else {
        appDataPath += "/" + orgName;
    }

    return appDataPath + "/" + appName;
}
