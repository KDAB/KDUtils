/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "win32_keyboard_map.h"

#include <KDFoundation/constexpr_sort.h>

#include <windows.h>

#include <array>

namespace KDGui {
constexpr std::array<Key, 512> scanCodeTable = {
    /* clang-format off */
    /* 0x000 - 0x007 */ Key_Unknown, Key_Escape, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6,
    /* 0x008 - 0x00f */ Key_7, Key_8, Key_9, Key_0, Key_Minus, Key_Equal, Key_Backspace, Key_Tab,
    /* 0x010 - 0x017 */ Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Y, Key_U, Key_I,
    /* 0x018 - 0x01f */ Key_O, Key_P, Key_BracketLeft, Key_BracketRight, Key_Enter, Key_LeftControl, Key_A, Key_S,
    /* 0x020 - 0x027 */ Key_D, Key_F, Key_G, Key_H, Key_J, Key_K, Key_L, Key_Semicolon,
    /* 0x028 - 0x02f */ Key_Apostrophe, Key_QuoteLeft, Key_LeftShift, Key_Backslash, Key_Z, Key_X, Key_C, Key_V,
    /* 0x030 - 0x037 */ Key_B, Key_N, Key_M, Key_Comma, Key_Period, Key_Slash, Key_RightShift, Key_NumPad_Multiply,
    /* 0x038 - 0x03f */ Key_LeftAlt, Key_Space, Key_CapsLock, Key_F1, Key_F2, Key_F3, Key_F4, Key_F5,
    /* 0x040 - 0x047 */ Key_F6, Key_F7, Key_F8, Key_F9, Key_F10, Key_Pause, Key_ScrollLock, Key_NumPad_7,
    /* 0x048 - 0x04f */ Key_NumPad_8, Key_NumPad_9, Key_NumPad_Subtract, Key_NumPad_4, Key_NumPad_5, Key_NumPad_6, Key_NumPad_Add, Key_NumPad_1,
    /* 0x050 - 0x057 */ Key_NumPad_2, Key_NumPad_3, Key_NumPad_0, Key_NumPad_Decimal, Key_Unknown, Key_Unknown, Key_Unknown, Key_F11,
    /* 0x058 - 0x05f */ Key_F12, Key_NumPad_Equal, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x060 - 0x067 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_F13, Key_F14, Key_F15, Key_F16,
    /* 0x068 - 0x06f */ Key_F17, Key_F18, Key_F19, Key_F20, Key_F21, Key_F22, Key_F23, Key_Unknown,
    /* 0x070 - 0x077 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_F24, Key_Unknown,
    /* 0x078 - 0x07f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x080 - 0x087 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x088 - 0x08f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x090 - 0x097 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x098 - 0x09f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0a0 - 0x0a7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0a8 - 0x0af */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0b0 - 0x0b7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0b8 - 0x0bf */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0c0 - 0x0c7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0c8 - 0x0cf */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0d0 - 0x0d7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0d8 - 0x0df */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0e0 - 0x0e7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0e8 - 0x0ef */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0f0 - 0x0f7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x0f8 - 0x0ff */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x100 - 0x107 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x108 - 0x10f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x110 - 0x117 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x118 - 0x11f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_NumPad_Enter, Key_RightControl, Key_Unknown, Key_Unknown,
    /* 0x120 - 0x127 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x128 - 0x12f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x130 - 0x137 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_NumPad_Divide, Key_Unknown, Key_PrintScreen,
    /* 0x138 - 0x13f */ Key_RightAlt, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x140 - 0x147 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_NumLock, Key_Pause, Key_Home,
    /* 0x148 - 0x14f */ Key_Up, Key_PageUp, Key_Unknown, Key_Left, Key_Unknown, Key_Right, Key_Unknown, Key_End,
    /* 0x150 - 0x157 */ Key_Down, Key_PageDown, Key_Insert, Key_Delete, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x158 - 0x15f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_LeftSuper, Key_RightSuper, Key_Menu, Key_Unknown, Key_Unknown,
    /* 0x160 - 0x167 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x168 - 0x16f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x170 - 0x177 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x178 - 0x17f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x180 - 0x187 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x188 - 0x18f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x190 - 0x197 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x198 - 0x19f */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1a0 - 0x1a7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1a8 - 0x1af */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1b0 - 0x1b7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1b8 - 0x1bf */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1c0 - 0x1c7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1c8 - 0x1cf */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1d0 - 0x1d7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1d8 - 0x1df */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1e0 - 0x1e7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1e8 - 0x1ef */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1f0 - 0x1f7 */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* 0x1f8 - 0x1ff */ Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown, Key_Unknown,
    /* clang-format on */
};

Key windowsScanCodeToKey(unsigned scanCode)
{
    if (scanCode >= scanCodeTable.size())
        return Key_Unknown;
    return scanCodeTable[scanCode];
}

} // namespace KDGui
