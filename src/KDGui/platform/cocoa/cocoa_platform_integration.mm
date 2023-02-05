/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "cocoa_platform_integration.h"
#include "cocoa_platform_integration_impl.h"

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
