/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_integration.h>

#include <KDGui/abstract_platform_window.h>

namespace KDGui {

class Window;
class AbstractClipboard;

class AbstractGuiPlatformIntegration : public KDFoundation::AbstractPlatformIntegration
{
public:
    std::unique_ptr<AbstractPlatformWindow> createPlatformWindow(Window *window)
    {
        return std::unique_ptr<AbstractPlatformWindow>(this->createPlatformWindowImpl(window));
    }

    virtual AbstractClipboard *clipboard() = 0;

private:
    virtual AbstractPlatformWindow *createPlatformWindowImpl(Window *window) = 0;
};

} // namespace KDGui
