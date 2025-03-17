/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_integration.h>

#include <KDFoundation/core_application.h>
#include "linux_platform_integration.h"

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

std::string LinuxPlatformIntegration::applicationDataPath(const CoreApplication &app) const
{
    return linuxAppDataPath(app);
}

std::string LinuxPlatformIntegration::assetsDataPath(const CoreApplication &) const
{
    return CoreApplication::applicationDir().parent().absoluteFilePath("assets");
}

std::string KDFoundation::LinuxPlatformIntegration::linuxAppDataPath(const CoreApplication &app)
{
    auto homePath = secure_getenv("HOME");
    auto appDataPath = std::string(homePath ? homePath : "") + "/.local/share";

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
