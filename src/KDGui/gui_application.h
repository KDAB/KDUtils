/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/core_application.h>

#include <KDGui/abstract_gui_platform_integration.h>
#include <KDGui/kdgui_global.h>

namespace KDGui {

class KDGUI_API GuiApplication : public KDFoundation::CoreApplication
{
public:
    /// @warning if you want to use custom loggers (@see KDUtils::logger::setLoggerFactory ) you
    //  should call it *before* creation of both this class and any platform integration.
    GuiApplication(std::unique_ptr<AbstractGuiPlatformIntegration> &&platformIntegration = {});

    static inline GuiApplication *instance() { return static_cast<GuiApplication *>(ms_application); }

    AbstractGuiPlatformIntegration *guiPlatformIntegration()
    {
        return dynamic_cast<AbstractGuiPlatformIntegration *>(platformIntegration());
    }
};

} // namespace KDGui
