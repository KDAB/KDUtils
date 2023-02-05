/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

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

private:
    KDFoundation::AbstractPlatformEventLoop *createPlatformEventLoopImpl() override;
    AbstractPlatformWindow *createPlatformWindowImpl(Window *window) override;

    std::unique_ptr<CocoaPlatformIntegrationImpl> m_impl;
};

} // namespace KDGui
