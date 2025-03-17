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

KDUtils::Dir CocoaPlatformIntegration::applicationDataDir(const KDFoundation::CoreApplication &app, bool) const
{
    return KDUtils::Dir(KDFoundation::MacOSPlatformIntegration::macAppDataPath(app));
}

KDUtils::Dir CocoaPlatformIntegration::assetsDataDir(const KDFoundation::CoreApplication &) const
{
    return KDUtils::Dir(KDFoundation::CoreApplication::applicationDir().parent().absoluteFilePath("assets"));
}
