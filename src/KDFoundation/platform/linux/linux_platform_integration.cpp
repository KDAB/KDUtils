/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/linux/linux_platform_integration.h>

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
