/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "macos_platform_integration.h"

#include <KDFoundation/core_application.h>

using namespace KDFoundation;

MacOSPlatformIntegration::MacOSPlatformIntegration() = default;

MacOSPlatformIntegration::~MacOSPlatformIntegration() = default;

MacOSPlatformEventLoop *MacOSPlatformIntegration::createPlatformEventLoopImpl()
{
    return new MacOSPlatformEventLoop;
}

KDUtils::Dir MacOSPlatformIntegration::standardDir(const CoreApplication &app, KDFoundation::StandardDir type) const
{
    switch (type) {
    case StandardDir::Application:
        return KDUtils::Dir::applicationDir();
    case StandardDir::ApplicationData:
    case StandardDir::ApplicationDataLocal:
        return KDUtils::Dir(KDFoundation::MacOSPlatformIntegration::macAppDataPath(app));
    case StandardDir::Assets:
        return KDUtils::Dir(KDUtils::Dir::applicationDir().parent().absoluteFilePath("assets"));
    default:
        SPDLOG_WARN("Unsupported standard directory requested");
        return {};
    }
}

std::string KDFoundation::MacOSPlatformIntegration::macAppDataPath(const CoreApplication &app)
{
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - there is no secure_getenv on MacOS
    auto homePath = std::getenv("HOME");
    auto appDataPath = std::string(homePath ? homePath : "") + "/Library/Application Support";

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
