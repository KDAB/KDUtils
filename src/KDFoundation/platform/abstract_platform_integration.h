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

#include <KDUtils/dir.h>

#include <memory>

namespace KDFoundation {

class CoreApplication;

enum class StandardDir {
    Application, // Directory containing the application executable
    ApplicationData, // Directory containing data files specific to this application
    ApplicationDataLocal, // Directory containing local or internal data files specific to this application
    Assets // Directory containing assets bundled with the application package
};

class KDFOUNDATION_API AbstractPlatformIntegration
{
public:
    virtual ~AbstractPlatformIntegration() { }

    /**
     * Finalize initialization of the platform integration.
     *
     * @warning This is called before CoreApplication and its subclasses are fully initialized.
     */
    virtual void init() { }
    std::unique_ptr<AbstractPlatformEventLoop> createPlatformEventLoop()
    {
        return std::unique_ptr<AbstractPlatformEventLoop>(this->createPlatformEventLoopImpl());
    }

    // Return a directory, at one of the standard directory locations
    virtual KDUtils::Dir standardDir(const CoreApplication &app, StandardDir type) const = 0;

private:
    /**
     * Create and return a platform event loop.
     *
     * @warning Might be called before @e AbstractPlatformIntegration::init.
     */
    virtual AbstractPlatformEventLoop *createPlatformEventLoopImpl() = 0;
};

} // namespace KDFoundation
