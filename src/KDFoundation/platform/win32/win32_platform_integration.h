/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_integration.h>
#include <KDFoundation/platform/win32/win32_platform_event_loop.h>
#include <KDFoundation/kdfoundation_global.h>

namespace KDFoundation {

class KDFOUNDATION_API Win32PlatformIntegration : public AbstractPlatformIntegration
{
public:
    Win32PlatformIntegration();
    ~Win32PlatformIntegration() override;

    std::string applicationDataPath(const CoreApplication &app) const override;
    std::string assetsDataPath(const CoreApplication &app) const override;

    static std::string windowsAppDataPath(const CoreApplication &app);

private:
    Win32PlatformEventLoop *createPlatformEventLoopImpl() override;
};

} // namespace KDFoundation
