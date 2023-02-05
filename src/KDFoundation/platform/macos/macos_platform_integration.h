/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_integration.h>
#include <KDFoundation/platform/macos/macos_platform_event_loop.h>
#include <KDFoundation/kdfoundation_global.h>

namespace KDFoundation {

class KDFOUNDATION_API MacOSPlatformIntegration : public AbstractPlatformIntegration
{
public:
    MacOSPlatformIntegration();
    ~MacOSPlatformIntegration() override;

private:
    MacOSPlatformEventLoop *createPlatformEventLoopImpl() override;
};

} // namespace KDFoundation
