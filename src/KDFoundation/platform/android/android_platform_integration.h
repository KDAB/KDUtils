/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_integration.h>

#include <string_view>

#ifdef __cplusplus
extern "C" {
#endif
struct android_app;
#ifdef __cplusplus
}
#endif

namespace KDFoundation {

class AndroidPlatformEventLoop;

class KDFOUNDATION_API AndroidPlatformIntegration : public AbstractPlatformIntegration
{
public:
    AndroidPlatformIntegration();
    ~AndroidPlatformIntegration() override;

    static android_app *s_androidApp;

    std::string applicationDataPath(const CoreApplication &app) const override;
    std::string assetsDataPath(const CoreApplication &app) const override;

    // Assets must be dealt with separately from regular files on Android, so we require a special path indicator
    static constexpr std::string_view assetsDirPathIndicator = "ANDROID_ASSETS_DIR";

private:
    AbstractPlatformEventLoop *createPlatformEventLoopImpl() override;
};

} // namespace KDFoundation
