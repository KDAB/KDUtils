/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/win32/win32_platform_integration.h>

#include <KDFoundation/core_application.h>
#include <KDUtils/logging.h>

#include <windows.h>
#include <shlobj.h>

#include <string>
#include <locale>
#include <codecvt>

using namespace KDFoundation;

Win32PlatformIntegration::Win32PlatformIntegration() = default;
Win32PlatformIntegration::~Win32PlatformIntegration() = default;

std::string Win32PlatformIntegration::windowsAppDataPath(const CoreApplication &app)
{
    std::string appDataPath;
    PWSTR path = nullptr;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path))) {
        // Convert the retrieved path (UTF-16) to a UTF-8 std::string
        const std::wstring widePath(path);
        const auto multibyteLength = WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), static_cast<int>(widePath.size()), nullptr, 0, nullptr, nullptr);
        appDataPath = std::string(multibyteLength, 0);
        WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), static_cast<int>(widePath.size()), appDataPath.data(), multibyteLength, nullptr, nullptr);

        // Free the system memory allocated for the path
        CoTaskMemFree(path);
    }
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
}

std::string Win32PlatformIntegration::applicationDataPath(const CoreApplication &app) const
{
    return windowsAppDataPath(app);
}

std::string Win32PlatformIntegration::assetsDataPath(const CoreApplication &) const
{
    return CoreApplication::applicationDir().parent().absoluteFilePath("assets");
}

Win32PlatformEventLoop *Win32PlatformIntegration::createPlatformEventLoopImpl()
{
    return new Win32PlatformEventLoop();
}
