/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "cocoa_platform_integration_impl.h"
#include "cocoa_platform_window.h"
#include <KDFoundation/platform/macos/macos_platform_event_loop.h>

#import <Cocoa/Cocoa.h>

#include <KDFoundation/core_application.h>

using namespace KDGui;

static void createMenuBar()
{
    id menu = [[NSMenu alloc] init];
    [NSApp setMainMenu:menu];

    id appMenuItem = [[NSMenuItem alloc] init];
    [menu addItem:appMenuItem];

    id appName = [[NSProcessInfo processInfo] processName];

    id appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];
    auto quitItem = [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                                       action:@selector(terminate:)
                                keyEquivalent:@"q"];
    quitItem.target = [NSApplication sharedApplication];
}

@interface KDGuiApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation KDGuiApplicationDelegate
- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    // set up menubar between sharedApplication and finishLaunching
    createMenuBar();
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    KDFoundation::MacOSPlatformEventLoop::postEmptyEvent();
    // we'll run the event loop manually from now on
    [NSApp stop:nil];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    KDFoundation::CoreApplication::instance()->quit();
    return NSTerminateCancel;
}
@end

CocoaPlatformIntegrationImpl::CocoaPlatformIntegrationImpl()
{
    @autoreleasepool {
        [NSApplication sharedApplication];

        m_delegate = [[KDGuiApplicationDelegate alloc] init];
        [NSApp setDelegate:m_delegate];

        if (![[NSRunningApplication currentApplication] isFinishedLaunching]) {
            // run event loop until the application is done launching
            [NSApp run];
        }

        // make us a proper UI application with a menubar
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    }
}

CocoaPlatformIntegrationImpl::~CocoaPlatformIntegrationImpl()
{
    @autoreleasepool {
        [NSApp setDelegate:nil];
        [m_delegate release];
        m_delegate = nil;
    }
}

AbstractPlatformWindow *CocoaPlatformIntegrationImpl::createPlatformWindow(Window *window)
{
    return new CocoaPlatformWindow(window);
}

KDFoundation::AbstractPlatformEventLoop *CocoaPlatformIntegrationImpl::createPlatformEventLoop()
{
    return new KDFoundation::MacOSPlatformEventLoop;
}
