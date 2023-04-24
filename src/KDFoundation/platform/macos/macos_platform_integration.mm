/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "macos_platform_integration.h"

using namespace KDFoundation;

MacOSPlatformIntegration::MacOSPlatformIntegration() = default;

MacOSPlatformIntegration::~MacOSPlatformIntegration() = default;

MacOSPlatformEventLoop *MacOSPlatformIntegration::createPlatformEventLoopImpl()
{
    return new MacOSPlatformEventLoop;
}
