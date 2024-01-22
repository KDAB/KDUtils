/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/core_application.h>
#include <KDFoundation/event.h>

#include <KDUtils/logging.h>

#include <KDGui/platform/win32/win32_platform_window.h>
#include <KDGui/window.h>
#include <KDGui/platform/win32/win32_gui_platform_integration.h>
#include <KDGui/platform/win32/win32_utils.h>
#include <KDGui/platform/win32/win32_keyboard_map.h>
#include <KDGui/gui_events.h>

#include <windowsx.h> // for GET_X_LPARAM
#include <hidusage.h>

using namespace KDFoundation;
using namespace KDGui;

namespace {
constexpr WCHAR *KDGuiWindowClassName = L"KDGuiWindowClass";
constexpr WCHAR *KDGuiPlatformWindowProperty = L"KDGuiPlatformWindow";

KeyboardModifiers getKeyboardModifiers()
{
    KeyboardModifiers modifiers = Mod_NoModifiers;
    if (GetKeyState(VK_SHIFT) & 0x8000)
        modifiers |= Mod_Shift;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        modifiers |= Mod_Control;
    if (GetKeyState(VK_MENU) & 0x8000)
        modifiers |= Mod_Alt;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        modifiers |= Mod_Logo;
    if (GetKeyState(VK_CAPITAL) & 1)
        modifiers |= Mod_CapsLock;
    if (GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= Mod_NumLock;
    return modifiers;
}

LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto *platformWindow = reinterpret_cast<Win32PlatformWindow *>(GetProp(hwnd, KDGuiPlatformWindowProperty));
    if (!platformWindow)
        return DefWindowProc(hwnd, message, wParam, lParam);

    const auto handleMousePress = [platformWindow, lParam](MouseButton button) {
        platformWindow->handleMousePress(GetMessageTime(), button, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    };

    const auto handleMouseRelease = [platformWindow, lParam](MouseButton button) {
        platformWindow->handleMouseRelease(GetMessageTime(), button, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    };

    constexpr const auto ScanCodeMask = 0x1ff; // 0-7: scancode; 8: extended

    switch (message) {
    case WM_SIZE: {
        const auto width = LOWORD(lParam);
        const auto height = HIWORD(lParam);
        platformWindow->handleResize(width, height);
        return 0;
    }
    case WM_CLOSE: {
        platformWindow->window()->visible = false;
        return 0;
    }
    case WM_LBUTTONDOWN: {
        handleMousePress(LeftButton);
        return 0;
    }
    case WM_MBUTTONDOWN: {
        handleMousePress(MiddleButton);
        return 0;
    }
    case WM_RBUTTONDOWN: {
        handleMousePress(RightButton);
        return 0;
    }
    case WM_LBUTTONUP: {
        handleMouseRelease(LeftButton);
        return 0;
    }
    case WM_MBUTTONUP: {
        handleMouseRelease(MiddleButton);
        return 0;
    }
    case WM_RBUTTONUP: {
        handleMouseRelease(RightButton);
        return 0;
    }
    case WM_MOUSEMOVE: {
        // We do not process legacy mouse move events if the window is set to
        // raw mouse input
        if (!platformWindow->isRawMouseInputEnabled())
            platformWindow->handleMouseMove(GetMessageTime(), MouseButton::NoButton, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }
    case WM_MOUSEWHEEL: {
        platformWindow->handleMouseWheel(GetMessageTime(), 0, GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;
    }
    case WM_MOUSEHWHEEL: {
        platformWindow->handleMouseWheel(GetMessageTime(), -GET_WHEEL_DELTA_WPARAM(wParam), 0);
        return 0;
    }
    case WM_KEYDOWN: {
        const auto scanCode = (static_cast<unsigned>(lParam) >> 16) & ScanCodeMask;
        platformWindow->handleKeyPress(GetMessageTime(), scanCode, windowsScanCodeToKey(scanCode), getKeyboardModifiers());
        return 0;
    }
    case WM_KEYUP: {
        const auto scanCode = (static_cast<unsigned>(lParam) >> 16) & ScanCodeMask;
        platformWindow->handleKeyRelease(GetMessageTime(), scanCode, windowsScanCodeToKey(scanCode), getKeyboardModifiers());
        return 0;
    }
    case WM_CHAR: {
        const auto characterCode = static_cast<wchar_t>(wParam);
        const auto text = wideStringToUtf8(std::wstring_view(&characterCode, 1));
        platformWindow->handleTextInput(text);
        return 0;
    }
    // We need to handle the WM_SETCURSOR case because if we don't, the DefWindowProc fallback
    // will keep resetting the cursor to the default cursor set on the window class of this window
    // which defaults to the usual arrow cursor. And it nicely does that every time the mouse moves.
    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            platformWindow->updateCursor();
            return true;
        }
        break;
    }
    case WM_INPUT: {
        HRAWINPUT rawInput = (HRAWINPUT)lParam;
        platformWindow->processRawInput(rawInput);

        // Let the default handler process this too
        break;
    }
    default:
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
constexpr DWORD WindowExStyle = 0;

} // namespace

Win32PlatformWindow::Win32PlatformWindow(Win32GuiPlatformIntegration *platformIntegration, Window *window)
    : AbstractPlatformWindow(window, AbstractPlatformWindow::Type::Win32)
    , m_platformIntegration(platformIntegration)
    , m_handle(nullptr)
{
    m_logger = m_platformIntegration->logger();
}

Win32PlatformWindow::~Win32PlatformWindow()
{
    destroy();
    delete m_rawInputData;
    m_rawInputData = nullptr;
    m_rawInputDataSize = 0;
}

bool Win32PlatformWindow::create()
{
    if (m_handle)
        return false;

    m_platformIntegration->registerWindowClass(KDGuiWindowClassName, CS_HREDRAW | CS_VREDRAW | CS_OWNDC, windowProc);

    const auto title = utf8StringToWide(m_window->title.get());
    const int x = CW_USEDEFAULT;
    const int y = CW_USEDEFAULT;

    auto rect = RECT{ 0, 0, static_cast<LONG>(m_window->width.get()), static_cast<LONG>(m_window->height.get()) };
    AdjustWindowRectEx(&rect, WindowStyle, FALSE, WindowExStyle);
    const auto width = rect.right - rect.left;
    const auto height = rect.bottom - rect.top;

    m_handle = CreateWindowEx(WindowExStyle, KDGuiWindowClassName, title.c_str(), WindowStyle, x, y, width, height,
                              nullptr, // no parent or owner window
                              nullptr, // no class menu
                              GetModuleHandle(nullptr),
                              nullptr);
    if (!m_handle) {
        SPDLOG_LOGGER_CRITICAL(m_logger, "Failed to create native window: {}", windowsErrorMessage(GetLastError()));
        return false;
    }

    SetProp(m_handle, KDGuiPlatformWindowProperty, this);

    return true;
}

bool Win32PlatformWindow::destroy()
{
    if (m_handle) {
        RemoveProp(m_handle, KDGuiPlatformWindowProperty);
        DestroyWindow(m_handle);
        m_handle = nullptr;
    }
    return true;
}

bool Win32PlatformWindow::isCreated()
{
    return m_handle != nullptr;
}

void Win32PlatformWindow::map()
{
    ShowWindow(m_handle, SW_SHOWNA);
}

void Win32PlatformWindow::unmap()
{
    ShowWindow(m_handle, SW_HIDE);
}

Position Win32PlatformWindow::queryCursorPosition() const
{
    Position cursorPos{ 0, 0 };
    POINT pos;
    if (GetCursorPos(&pos)) {
        ScreenToClient(m_handle, &pos);
        cursorPos.x = pos.x;
        cursorPos.y = pos.y;
    }
    return cursorPos;
}

Position Win32PlatformWindow::queryWindowSize() const
{
    Position windowSize{ 0, 0 };
    RECT rect;
    if (GetClientRect(m_handle, &rect)) {
        windowSize.x = rect.right;
        windowSize.y = rect.bottom;
    }
    return windowSize;
}

void Win32PlatformWindow::disableCursor()
{
    // Remember where the cursor is
    m_cursorRestorePosition = queryCursorPosition();

    // Hide the cursor
    m_cursorMode = CursorMode::Disabled;
    updateCursor();

    // Move cursor to centre of the window
    const auto windowSize = queryWindowSize();
    const Position center{ static_cast<int64_t>(windowSize.x / 2), static_cast<int64_t>(windowSize.y / 2) };
    POINT pos{ static_cast<LONG>(center.x), static_cast<LONG>(center.y) };
    ClientToScreen(m_handle, &pos);
    SetCursorPos(pos.x, pos.y);
    m_previousWarpedCursorPosition = center;

    // Grab the cursor
    RECT clipRect;
    GetClientRect(m_handle, &clipRect);
    ClientToScreen(m_handle, (POINT *)&clipRect.left); // Map top-left to screen coords
    ClientToScreen(m_handle, (POINT *)&clipRect.right); // Map bottom-right to screen coords
    ClipCursor(&clipRect);
}

void Win32PlatformWindow::enableCursor()
{
    // Ungrab the cursor
    ClipCursor(NULL);

    // Restore the cursor position
    POINT pos{ static_cast<LONG>(m_cursorRestorePosition.x), static_cast<LONG>(m_cursorRestorePosition.y) };
    ClientToScreen(m_handle, &pos);
    SetCursorPos(pos.x, pos.y);

    // Reset the default cursor (inherit from parent window)
    m_cursorMode = CursorMode::Normal;
    updateCursor();
}

void Win32PlatformWindow::updateCursor()
{
    if (m_cursorMode == CursorMode::Normal)
        SetCursor(LoadCursorW(NULL, IDC_ARROW));
    else
        SetCursor(NULL);
}

void Win32PlatformWindow::enableRawMouseInput()
{
    const RAWINPUTDEVICE rawInputDevice = {
        HID_USAGE_PAGE_GENERIC,
        HID_USAGE_GENERIC_MOUSE,
        0,
        m_handle
    };
    const auto result = RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice));
    // NB: We will now receive WM_INPUT messages
    if (result) {
        m_rawMouseInputEnabled = true;
        SPDLOG_LOGGER_DEBUG(m_logger, "Enabled raw mouse input");
    } else {
        SPDLOG_LOGGER_WARN(m_logger, "Failed to enable raw mouse input");
    }
}

void Win32PlatformWindow::disableRawMouseInput()
{
    const RAWINPUTDEVICE rawInputDevice = {
        HID_USAGE_PAGE_GENERIC,
        HID_USAGE_GENERIC_MOUSE,
        RIDEV_REMOVE,
        NULL
    };
    const auto result = RegisterRawInputDevices(&rawInputDevice, 1, sizeof(rawInputDevice));
    // NB: We will no longer receive WM_INPUT messages
    if (result) {
        m_rawMouseInputEnabled = false;
        SPDLOG_LOGGER_DEBUG(m_logger, "Disabled raw mouse input");
    } else {
        auto error = GetLastError();
        SPDLOG_LOGGER_WARN(m_logger, "Failed to disable raw mouse input. Error = {}", error);
    }
}

void KDGui::Win32PlatformWindow::grabMouse()
{
    if (m_handle) {
        SetCapture(m_handle);
    }
}
void KDGui::Win32PlatformWindow::releaseMouse()
{
    ReleaseCapture();
}
void Win32PlatformWindow::setTitle(const std::string &title)
{
    SetWindowText(m_handle, utf8StringToWide(title).c_str());
}

void Win32PlatformWindow::setSize(uint32_t width, uint32_t height)
{
    auto rect = RECT{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    AdjustWindowRectEx(&rect, WindowStyle, FALSE, WindowExStyle);
    SetWindowPos(m_handle, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);
}

void Win32PlatformWindow::handleResize(uint32_t width, uint32_t height)
{
    ResizeEvent ev{ width, height };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleMousePress(uint32_t timestamp, MouseButton button, int16_t xPos, int16_t yPos)
{
    m_mouseButtons.setFlag(button);
    grabMouse();
    MousePressEvent ev{ timestamp, button, m_mouseButtons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleMouseRelease(uint32_t timestamp, MouseButton button, int16_t xPos, int16_t yPos)
{
    m_mouseButtons.setFlag(button, false);
    if (m_mouseButtons == MouseButton::NoButton) {
        releaseMouse();
    }
    MouseReleaseEvent ev{ timestamp, button, m_mouseButtons, xPos, yPos };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleMouseMove(uint32_t timestamp, MouseButton /* button */, int64_t xPos, int64_t yPos)
{
    Position pos{ xPos, yPos };
    const bool processMouseMove = pos != m_previousWarpedCursorPosition;

    if (processMouseMove && m_cursorMode == CursorMode::Disabled) {
        const Position delta = pos - m_previousCursorPosition;
        pos = window()->cursorPosition.get() + delta;
    }

    m_previousCursorPosition = Position(xPos, yPos);

    // Re-center the cursor if in Disabled cursor mode
    if (m_cursorMode == CursorMode::Disabled) {
        const auto windowSize = queryWindowSize();
        const Position center{ static_cast<int64_t>(windowSize.x / 2), static_cast<int64_t>(windowSize.y / 2) };
        if (m_previousCursorPosition != center) {
            POINT pos{ static_cast<LONG>(center.x), static_cast<LONG>(center.y) };
            ClientToScreen(m_handle, &pos);
            SetCursorPos(pos.x, pos.y);
            m_previousWarpedCursorPosition = center;
        }
    }

    if (processMouseMove) {
        MouseMoveEvent ev{ timestamp, m_mouseButtons, pos.x, pos.y };
        CoreApplication::instance()->sendEvent(m_window, &ev);
    }
}

void Win32PlatformWindow::handleMouseWheel(uint32_t timestamp, int32_t xDelta, int32_t yDelta)
{
    MouseWheelEvent ev{ timestamp, xDelta, yDelta };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleKeyPress(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyPressEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleKeyRelease(uint32_t timestamp, uint8_t nativeKeyCode, Key key, KeyboardModifiers modifiers)
{
    KeyReleaseEvent ev{ timestamp, nativeKeyCode, key, modifiers };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::handleTextInput(const std::string &str)
{
    TextInputEvent ev{ str };
    CoreApplication::instance()->sendEvent(m_window, &ev);
}

void Win32PlatformWindow::processRawInput(HRAWINPUT rawInput)
{
    // We only process raw input when we have grabbed and hidden the cursor
    // TODO: We may also want to do so when we just have a normal cursor - think
    //       RTS type games. This may be with a grabbed or ungrabbed cursor.
    //       Again, this edge-scrolling in RTS games.
    //
    // We should probably have separate settings for:
    //
    // * Grabbing/restricting cursor movement
    // * Hiding/showing the cursor
    // * Using raw input vs regular input events
    if (m_cursorMode != CursorMode::Disabled)
        return;

    // Query how much storage we need to hold the raw input data
    UINT requiredSize{ 0 };
    GetRawInputData(rawInput, RID_INPUT, nullptr, &requiredSize, sizeof(RAWINPUTHEADER));
    if (static_cast<UINT>(m_rawInputDataSize) < requiredSize) {
        delete m_rawInputData;
        auto rawData = new char[requiredSize];
        m_rawInputData = reinterpret_cast<RAWINPUT *>(rawData);
        m_rawInputDataSize = static_cast<size_t>(requiredSize);
    }

    if (GetRawInputData(rawInput, RID_INPUT, m_rawInputData, &requiredSize, sizeof(RAWINPUTHEADER)) != requiredSize) {
        SPDLOG_LOGGER_ERROR(m_logger, "GetRawInputData did not return data of the expected size");
        return;
    }

    Position pos{ 0, 0 };
    if (m_rawInputData->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) {
        pos.x = m_rawInputData->data.mouse.lLastX;
        pos.y = m_rawInputData->data.mouse.lLastY;
        SPDLOG_LOGGER_TRACE(m_logger, "RawInput (Absolute): delta = {}, {}", pos.x, pos.y);
    } else {
        pos.x = m_previousCursorPosition.x + m_rawInputData->data.mouse.lLastX;
        pos.y = m_previousCursorPosition.y + m_rawInputData->data.mouse.lLastY;
        SPDLOG_LOGGER_TRACE(m_logger, "RawInput (Relative): delta = {}, {}", pos.x, pos.y);
    }
    handleMouseMove(GetMessageTime(), NoButton, pos.x, pos.y);
}
