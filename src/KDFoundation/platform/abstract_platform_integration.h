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

class KDFOUNDATION_API AbstractPlatformIntegration
{
public:
    virtual ~AbstractPlatformIntegration() { }

    virtual void init() { }
    std::unique_ptr<AbstractPlatformEventLoop> createPlatformEventLoop()
    {
        return std::unique_ptr<AbstractPlatformEventLoop>(this->createPlatformEventLoopImpl());
    }

private:
    virtual AbstractPlatformEventLoop *createPlatformEventLoopImpl() = 0;
};

} // namespace KDFoundation
