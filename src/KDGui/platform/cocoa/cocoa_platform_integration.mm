/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "cocoa_platform_integration.h"
#include "cocoa_platform_integration_impl.h"

#include <KDFoundation/core_application.h>
#include <KDFoundation/platform/macos/macos_platform_integration.h>

using namespace KDGui;
using namespace KDFoundation;

CocoaPlatformIntegration::CocoaPlatformIntegration()
    : m_impl(new CocoaPlatformIntegrationImpl)
{
}

CocoaPlatformIntegration::~CocoaPlatformIntegration() = default;

KDFoundation::AbstractPlatformEventLoop *CocoaPlatformIntegration::createPlatformEventLoopImpl()
{
    return m_impl->createPlatformEventLoop();
}

AbstractPlatformWindow *CocoaPlatformIntegration::createPlatformWindowImpl(Window *window)
{
    return m_impl->createPlatformWindow(window);
}

KDUtils::Dir CocoaPlatformIntegration::standardDir(const KDFoundation::CoreApplication &app, KDFoundation::StandardDir type) const
{
    switch (type) {
    case StandardDir::Application:
        return KDUtils::Dir(KDUtils::Dir::applicationDir().path());
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
