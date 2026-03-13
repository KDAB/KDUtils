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
#include <filesystem>

using namespace KDFoundation;

Win32PlatformIntegration::Win32PlatformIntegration() = default;
Win32PlatformIntegration::~Win32PlatformIntegration() = default;

std::string Win32PlatformIntegration::windowsAppDataPath(const CoreApplication &app, bool local)
{
    std::filesystem::path appDataPath;
    PWSTR path = nullptr;
    const auto folderId = local ? FOLDERID_LocalAppData : FOLDERID_RoamingAppData;

    if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &path))) {
        // Convert the retrieved path (UTF-16) to a UTF-8 std::string
        const std::wstring widePath(path);
        const auto multibyteLength = WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), static_cast<int>(widePath.size()), nullptr, 0, nullptr, nullptr);
        auto appDataPathString = std::string(multibyteLength, 0);
        WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), static_cast<int>(widePath.size()), appDataPathString.data(), multibyteLength, nullptr, nullptr);
        appDataPath = appDataPathString;
        // Free the system memory allocated for the path
        CoTaskMemFree(path);
    }
    auto appName = app.applicationName();
    if (appName.empty()) {
        SPDLOG_CRITICAL("Application name is required to be set in order to generate an Application Data directory path");
        return {};
    }

    const auto orgName = app.organizationName();
    if (orgName.empty()) {
        SPDLOG_WARN("No Organization name - using only Application name for the directory");
    } else {
        appDataPath /= orgName;
    }

    return (appDataPath / appName).generic_u8string();
}

KDUtils::Dir Win32PlatformIntegration::standardDir(const KDFoundation::CoreApplication &app, KDFoundation::StandardDir type) const
{
    switch (type) {
    case StandardDir::Application:
        return KDUtils::Dir::applicationDir();
    case StandardDir::ApplicationData:
        return KDUtils::Dir(windowsAppDataPath(app, false));
    case StandardDir::ApplicationDataLocal:
        return KDUtils::Dir(windowsAppDataPath(app, true));
    case StandardDir::Assets:
        return KDUtils::Dir(KDUtils::Dir::applicationDir().parent().absoluteFilePath("assets"));
    default:
        SPDLOG_WARN("Unsupported standard directory requested");
        return {};
    }
}
Win32PlatformEventLoop *Win32PlatformIntegration::createPlatformEventLoopImpl()
{
    return new Win32PlatformEventLoop();
}
