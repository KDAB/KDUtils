/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "cocoa_platform_window.h"

#include <KDGui/window.h>
#include <KDGui/gui_events.h>

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>
#include <KDFoundation/logging.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

using namespace KDFoundation;
using namespace KDGui;

namespace {

KDGui::Key mapKeyCode(unsigned short keyCode, NSString *characters)
{
    KDGui::Key res = Key_Unknown;
    static std::vector<std::pair<unichar, KDGui::Key>> charCodeMap = {
        { '{', Key_BraceLeft },
        { '}', Key_BraceRight },
        { '!', Key_Exclamation },
        { '@', Key_At },
        { '&', Key_Ampersand },
        { '(', Key_ParenLeft },
        { ')', Key_ParenRight },
        { '-', Key_Minus },
        { '_', Key_Underscore },
        { '+', Key_Plus },
        { '=', Key_Equal },
        { '[', Key_BracketLeft },
        { ']', Key_BracketRight },
        { '"', Key_DoubleQuote },
        { '~', Key_AsciiTilde },
        { '~', Key_AsciiTilde },
        { '/', Key_Slash },
        { '?', Key_Question },
        { ',', Key_Comma },
        { '<', Key_Less },
        { '>', Key_Greater },
        { '.', Key_Period },
        { '%', Key_Percent },
        { '$', Key_Dollar },
        { '#', Key_HashSign },
        { '*', Key_NumPad_Multiply },
        { ':', Key_Colon },
        { ';', Key_Semicolon },
        { '|', Key_Bar },
        { '1', Key_1 },
        { '2', Key_2 },
        { '3', Key_3 },
        { '4', Key_4 },
        { '5', Key_5 },
        { '6', Key_6 },
        { '7', Key_7 },
        { '8', Key_8 },
        { '9', Key_9 },
        { '0', Key_0 },
        { '^', Key_AsciiCircum },
        { '\\', Key_Backslash },
        { '\'', Key_Apostrophe },
    };

    static std::vector<KDGui::Key> keyCodeTable = {
        Key_A,
        Key_S,
        Key_D,
        Key_F,
        Key_H,
        Key_G,
        Key_Z,
        Key_X,
        Key_C,
        Key_V,
        Key_Unknown, // 10
        Key_B,
        Key_Q,
        Key_W,
        Key_E,
        Key_R,
        Key_Y,
        Key_T,
        Key_1,
        Key_2,
        Key_3, // 20
        Key_4,
        Key_6,
        Key_5,
        Key_Equal,
        Key_9,
        Key_7,
        Key_Minus,
        Key_8,
        Key_0,
        Key_BracketRight, // 30
        Key_O,
        Key_U,
        Key_BracketLeft,
        Key_I,
        Key_P,
        Key_Enter,
        Key_L,
        Key_J,
        Key_Apostrophe,
        Key_K, // 40
        Key_Semicolon,
        Key_Slash,
        Key_Comma,
        Key_Backslash,
        Key_N,
        Key_M,
        Key_Period,
        Key_Tab,
        Key_Space,
        Key_QuoteLeft, // 50
        Key_Backspace,
        Key_Unknown, // 52
        Key_Escape,
    };

    if (characters.length) {
        // check for known characters
        unichar c = [characters characterAtIndex:0];
        for (const auto &p : charCodeMap) {
            if (p.first == c) {
                res = p.second;
                break;
            }
        }
    }
    if (res == Key_Unknown) {
        // check for known keycodes
        if (keyCode < keyCodeTable.size()) {
            res = keyCodeTable[keyCode];
        } else {
            // should we fold this in the table above?
            static std::vector<std::pair<unsigned short, KDGui::Key>> keyCodeMap = {
                { 67, Key_NumPad_Multiply },
                { 69, Key_NumPad_Add },
                { 75, Key_NumPad_Divide },
                { 75, Key_NumPad_Enter },
                { 78, Key_NumPad_Subtract },
                { 83, Key_NumPad_1 },
                { 84, Key_NumPad_2 },
                { 85, Key_NumPad_3 },
                { 86, Key_NumPad_4 },
                { 87, Key_NumPad_5 },
                { 88, Key_NumPad_6 },
                { 89, Key_NumPad_7 },
                { 91, Key_NumPad_8 },
                { 92, Key_NumPad_9 },
                { 115, Key_Home },
                { 116, Key_PageUp },
                { 116, Key_Delete },
                { 119, Key_End },
                { 121, Key_PageDown },
                { 123, Key_Left },
                { 124, Key_Right },
                { 125, Key_Down },
                { 122, Key_F1 },
                { 120, Key_F2 },
                { 99, Key_F3 },
                { 118, Key_F4 },
                { 96, Key_F5 },
                { 97, Key_F6 },
                { 98, Key_F7 },
                { 100, Key_F8 },
                { 101, Key_F9 },
                { 109, Key_F10 },
                { 103, Key_F11 },
                { 111, Key_F12 },
            };

            for (const auto &p : keyCodeMap) {
                if (p.first == keyCode) {
                    res = p.second;
                    break;
                }
            }
        }
    }

    NSLog(@"%hu : %@ : 0x%X", keyCode, characters, res);

    return res;
}

KDGui::KeyboardModifiers mapModifiers(NSEventModifierFlags flags)
{
    static std::vector<std::pair<KDGui::KeyboardModifier, NSEventModifierFlags>> modifierMap = {
        { Mod_Shift, NSEventModifierFlagShift },
        { Mod_Control, NSEventModifierFlagCommand },
        { Mod_Alt, NSEventModifierFlagOption },
        { Mod_Logo, NSEventModifierFlagControl },
        { Mod_CapsLock, NSEventModifierFlagCapsLock },
        { Mod_NumLock, NSEventModifierFlagNumericPad },
    };

    KDGui::KeyboardModifiers res = Mod_NoModifiers;
    for (auto &p : modifierMap) {
        if (flags & p.second)
            res |= p.first;
    }

    return res;
}

} // namespace

@interface KDGuiWindowDelegate : NSObject {
    CocoaPlatformWindow *m_platformWindow;
}

- (instancetype)initWithPlatformWindow:(CocoaPlatformWindow *)platformWindow;
@end

@implementation KDGuiWindowDelegate
- (instancetype)initWithPlatformWindow:(CocoaPlatformWindow *)platformWindow
{
    self = [super init];
    if (self != nil)
        m_platformWindow = platformWindow;
    return self;
}

- (void)windowWillClose:(NSNotification *)notification
{
    m_platformWindow->window()->visible = false;
}

- (void)windowDidResize:(NSNotification *)notification
{
    const NSRect contentRect = [m_platformWindow->nativeWindow() frame];
    const auto width = static_cast<uint32_t>(contentRect.size.width);
    const auto height = static_cast<uint32_t>(contentRect.size.height);
    m_platformWindow->handleResize(width, height);
}
@end

@interface KDGuiView : NSView {
    CocoaPlatformWindow *m_platformWindow;
    MouseButtons m_mouseButtons;
}

- (instancetype)initWithPlatformWindow:(CocoaPlatformWindow *)platformWindow;
- (NSPoint)localPosition:(NSPoint)eventLocation;
@end

@implementation KDGuiView
- (instancetype)initWithPlatformWindow:(CocoaPlatformWindow *)platformWindow
{
    self = [super init];
    if (self != nil) {
        m_platformWindow = platformWindow;
        m_mouseButtons = NoButton;
    }
    return self;
}

- (NSPoint)localPosition:(NSPoint)eventLocation
{
    const NSPoint position = [self convertPoint:eventLocation fromView:nil];
    const NSRect contentRect = [self frame];
    return NSMakePoint(position.x, contentRect.size.height - position.y);
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)timeStamp
{
    return YES;
}

- (uint32_t)timeStamp:(NSEvent *)event
{
    return uint32_t(event.timestamp * 1000.); // number of milliseconds since system booted up
}

- (void)mouseDown:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMousePress([self timeStamp:event], LeftButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons |= LeftButton;
}

- (void)mouseDragged:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseMove([self timeStamp:event], LeftButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
}

- (void)mouseUp:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseRelease([self timeStamp:event], LeftButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons.setFlag(LeftButton, false);
}

- (void)mouseMoved:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseMove([self timeStamp:event], m_mouseButtons, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
}

- (void)rightMouseDown:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMousePress([self timeStamp:event], RightButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons |= RightButton;
    [super rightMouseDown:event];
}

- (void)rightMouseDragged:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseMove([self timeStamp:event], RightButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
}

- (void)rightMouseUp:(NSEvent *)event
{
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseRelease([self timeStamp:event], RightButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons.setFlag(RightButton, false);
}

- (void)otherMouseDown:(NSEvent *)event
{
    const auto b = [NSEvent pressedMouseButtons];
    if ((b & 4) == 0) // 1 is LeftButton, 2 is RightButton, 4 is MiddleButton, others ignored
        return;
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMousePress([self timeStamp:event], MiddleButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons |= MiddleButton;
}

- (void)otherMouseDragged:(NSEvent *)event
{
    const auto b = [NSEvent pressedMouseButtons];
    if ((b & 4) == 0)
        return;
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMouseMove([self timeStamp:event], m_mouseButtons, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
}

- (void)otherMouseUp:(NSEvent *)event
{
    const auto b = [NSEvent pressedMouseButtons];
    if ((b & 4) == 0)
        return;
    const NSPoint pos = [self localPosition:[event locationInWindow]];
    m_platformWindow->handleMousePress([self timeStamp:event], MiddleButton, static_cast<int16_t>(pos.x), static_cast<int16_t>(pos.y));
    m_mouseButtons.setFlag(MiddleButton, false);
}

- (void)scrollWheel:(NSEvent *)event
{
    m_platformWindow->handleMouseWheel([self timeStamp:event], static_cast<int16_t>(event.deltaX), static_cast<int16_t>(event.deltaY));
}

- (void)keyDown:(NSEvent *)event
{
    m_platformWindow->handleKeyPress([self timeStamp:event], static_cast<uint8_t>(event.keyCode), mapKeyCode(event.keyCode, event.characters), mapModifiers(event.modifierFlags));
}

- (void)keyUp:(NSEvent *)event
{
    m_platformWindow->handleKeyRelease([self timeStamp:event], static_cast<uint8_t>(event.keyCode), mapKeyCode(event.keyCode, event.characters), mapModifiers(event.modifierFlags));
}

@end

CocoaPlatformWindow::CocoaPlatformWindow(Window *window)
    : AbstractPlatformWindow(window, AbstractPlatformWindow::Type::Cocoa)
{
}

CocoaPlatformWindow::~CocoaPlatformWindow() = default;

bool CocoaPlatformWindow::create()
{
    if (m_nativeWindow != nil)
        return false;

    @autoreleasepool {
        const NSRect contentRect = NSMakeRect(0, 0, m_window->width.get(), m_window->height.get());
        const NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
        m_nativeWindow = [[NSWindow alloc] initWithContentRect:contentRect
                                                     styleMask:styleMask
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
        if (m_nativeWindow == nil) {
            SPDLOG_CRITICAL("Failed to create native window");
            return false;
        }

        m_view = [[KDGuiView alloc] initWithPlatformWindow:this];
        m_delegate = [[KDGuiWindowDelegate alloc] initWithPlatformWindow:this];

        const auto &title = m_window->title.get();
        [m_nativeWindow setTitle:[NSString stringWithUTF8String:title.c_str()]];
        [m_nativeWindow setContentView:m_view];
        [m_nativeWindow setDelegate:m_delegate];
        [m_nativeWindow makeFirstResponder:m_view];
        [m_nativeWindow setAcceptsMouseMovedEvents:YES];
        [m_nativeWindow setRestorable:NO];

        [NSApp activateIgnoringOtherApps:YES];
    }

    return true;
}

bool CocoaPlatformWindow::destroy()
{
    @autoreleasepool {
        [m_nativeWindow setDelegate:nil];
        [m_delegate release];
        m_delegate = nil;

        [m_view release];
        m_view = nil;

        [m_nativeWindow close];
        m_nativeWindow = nil;
    }

    return true;
}

bool CocoaPlatformWindow::isCreated()
{
    return m_nativeWindow != nil;
}

void CocoaPlatformWindow::map()
{
    @autoreleasepool {
        [m_nativeWindow makeKeyAndOrderFront:nil];
    }
}

void CocoaPlatformWindow::unmap()
{
    @autoreleasepool {
        [m_nativeWindow orderOut:nil];
    }
}

void CocoaPlatformWindow::disableCursor()
{
    // TODO: Implement me!
}

void CocoaPlatformWindow::enableCursor()
{
    // TODO: Implement me!
}

void CocoaPlatformWindow::enableRawMouseInput()
{
    // TODO: Implement me!
}

void CocoaPlatformWindow::disableRawMouseInput()
{
    // TODO: Implement me!
}

void CocoaPlatformWindow::setTitle(const std::string &title)
{
    @autoreleasepool {
        [m_nativeWindow setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

void CocoaPlatformWindow::setSize(uint32_t /* width */, uint32_t /* height */)
{
    // TODO
}

void CocoaPlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    ResizeEvent ev{ width, height };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleMousePress(uint32_t timestamp, MouseButtons buttons, int16_t xPos, int16_t yPos)
{
    MousePressEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleMouseRelease(uint32_t timestamp, MouseButtons buttons, int16_t xPos, int16_t yPos)
{
    MouseReleaseEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleMouseMove(uint32_t timestamp, MouseButtons buttons, int64_t xPos, int64_t yPos)
{
    MouseMoveEvent ev{ timestamp, buttons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
{
    MouseWheelEvent ev{ timestamp, xDelta, yDelta };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyPressEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyReleaseEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void CocoaPlatformWindow::handleTextInput(const std::string &str)
{
    TextInputEvent ev{ str };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}
