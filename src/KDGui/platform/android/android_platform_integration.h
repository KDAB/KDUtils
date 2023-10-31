/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: BogDan Vatra <bogdan@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/abstract_gui_platform_integration.h>
#include <KDFoundation/logging.h>
#include <KDGui/kdgui_global.h>

#include <unordered_set>

#ifdef __cplusplus
extern "C" {
#endif
struct android_app;
#ifdef __cplusplus
}
#endif

namespace KDGui {

class KDGUI_API AndroidPlatformIntegration : public AbstractGuiPlatformIntegration
{
public:
    AndroidPlatformIntegration();
    void handleWindowResize();
    void registerPlatformWindow(AbstractPlatformWindow *window);
    void unregisterPlatformWindow(AbstractPlatformWindow *window);

    AbstractClipboard *clipboard() override
    {
        // TODO(android): Implement clipboard
        return nullptr;
    }

public:
    static android_app *s_androidApp;

private:
    KDFoundation::AbstractPlatformEventLoop *createPlatformEventLoopImpl() override;
    AbstractPlatformWindow *createPlatformWindowImpl(Window *window) override;

private:
    std::unordered_set<AbstractPlatformWindow *> m_windows;
};

} // namespace KDGui
