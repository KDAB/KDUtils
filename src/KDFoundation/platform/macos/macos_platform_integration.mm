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

KDUtils::Dir MacOSPlatformIntegration::applicationDataDir(const CoreApplication &app, bool) const
{
    return macAppDataPath(app);
}

KDUtils::Dir MacOSPlatformIntegration::assetsDataDir(const CoreApplication &) const
{
    return KDUtils::Dir(CoreApplication::applicationDir().parent().absoluteFilePath("assets"));
}

std::string KDFoundation::MacOSPlatformIntegration::macAppDataPath(const CoreApplication &app)
{
    // NOLINTNEXTLINE(concurrency-mt-unsafe) - there is no secure_getenv on MacOS
    auto homePath = std::getenv("HOME");
    auto appDataPath = std::string(homePath ? homePath : "") + "/Library/Application Support";

    auto appName = app.applicationName();
    if (appName.empty()) {
        SPDLOG_CRITICAL("Application name is required to be set in order to generate an Application Data directory path");
    }

    const auto orgName = app.organizationName();
    if (orgName.empty()) {
        SPDLOG_WARN("No Organization name - using only Application name for the directory");
    } else {
        appDataPath += "/" + orgName;
    }

    return appDataPath + "/" + appName;

    return {};
}
