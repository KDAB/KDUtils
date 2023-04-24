/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/win32/win32_platform_integration.h>

using namespace KDFoundation;

Win32PlatformIntegration::Win32PlatformIntegration() = default;
Win32PlatformIntegration::~Win32PlatformIntegration() = default;

Win32PlatformEventLoop *Win32PlatformIntegration::createPlatformEventLoopImpl()
{
    return new Win32PlatformEventLoop();
}
