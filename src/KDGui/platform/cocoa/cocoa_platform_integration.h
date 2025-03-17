/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_gui_platform_integration.h>
#include <KDGui/kdgui_global.h>

namespace KDGui {
class CocoaPlatformIntegrationImpl;
class Window;

class KDGUI_API CocoaPlatformIntegration : public AbstractGuiPlatformIntegration
{
public:
    CocoaPlatformIntegration();
    ~CocoaPlatformIntegration() override;

    AbstractClipboard *clipboard() override
    {
        // TODO(cocoa): Implement clipboard
        return nullptr;
    }

    KDUtils::Dir standardDir(const KDFoundation::CoreApplication &app, KDFoundation::StandardDir type) const override;

private:
    KDFoundation::AbstractPlatformEventLoop *createPlatformEventLoopImpl() override;
    AbstractPlatformWindow *createPlatformWindowImpl(Window *window) override;

    std::unique_ptr<CocoaPlatformIntegrationImpl> m_impl;
};

} // namespace KDGui
