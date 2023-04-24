/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_integration.h>
#include <KDFoundation/platform/linux/linux_platform_event_loop.h>

namespace KDFoundation {

class KDFOUNDATION_API LinuxPlatformIntegration : public AbstractPlatformIntegration
{
public:
    LinuxPlatformIntegration();
    ~LinuxPlatformIntegration() override;

private:
    LinuxPlatformEventLoop *createPlatformEventLoopImpl() override;
};

} // namespace KDFoundation
