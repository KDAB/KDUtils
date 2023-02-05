/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#import <Cocoa/Cocoa.h>

namespace KDFoundation {
class AbstractPlatformEventLoop;
}

namespace KDGui {

class Window;
class AbstractPlatformWindow;

class CocoaPlatformIntegrationImpl
{
public:
    CocoaPlatformIntegrationImpl();
    ~CocoaPlatformIntegrationImpl();

    KDFoundation::AbstractPlatformEventLoop *createPlatformEventLoop();
    AbstractPlatformWindow *createPlatformWindow(Window *window);

private:
    id m_delegate = nil;
};

} // namespace KDGui
