/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/kdfoundation_global.h>

#include <memory>

namespace KDFoundation {

class CoreApplication;

class KDFOUNDATION_API AbstractPlatformIntegration
{
public:
    virtual ~AbstractPlatformIntegration() { }

    virtual void init() { }
    std::unique_ptr<AbstractPlatformEventLoop> createPlatformEventLoop()
    {
        return std::unique_ptr<AbstractPlatformEventLoop>(this->createPlatformEventLoopImpl());
    }

    // Returns the path to the application's data directory.
    // Usually this depends on the application name and organization from the application.
    virtual std::string applicationDataPath(const CoreApplication &app) const = 0;

    // Returns the path to the application's asset directory.
    virtual std::string assetsDataPath(const CoreApplication &app) const = 0;

private:
    virtual AbstractPlatformEventLoop *createPlatformEventLoopImpl() = 0;
};

} // namespace KDFoundation
