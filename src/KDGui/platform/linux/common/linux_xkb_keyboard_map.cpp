/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xkb_keyboard_map.h"

#include <KDFoundation/constexpr_sort.h>

#include <xkbcommon/xkbcommon-keysyms.h>

#include <array>

namespace KDGui::xkb {

struct XkbToKeyEntry {
    xkb_keysym_t keysym;
    KDGui::Key key;

    constexpr bool operator<(XkbToKeyEntry const &other) const noexcept
    {
        return keysym < other.keysym;
    }
};

// This is compile time sorted so we can use std::lower_bound to perform a bisection
// search when mapping from xkb keys to KDGui keys.
constexpr auto keysymTable = KDFoundation::sort(
        std::array{
                XkbToKeyEntry{ XKB_KEY_space, Key_Space },
                XkbToKeyEntry{ XKB_KEY_exclam, Key_Exclamation },
                XkbToKeyEntry{ XKB_KEY_quotedbl, Key_DoubleQuote },
                XkbToKeyEntry{ XKB_KEY_numbersign, Key_HashSign },
                XkbToKeyEntry{ XKB_KEY_dollar, Key_Dollar },
                XkbToKeyEntry{ XKB_KEY_percent, Key_Percent },
                XkbToKeyEntry{ XKB_KEY_ampersand, Key_Ampersand },
                XkbToKeyEntry{ XKB_KEY_apostrophe, Key_Apostrophe },
                XkbToKeyEntry{ XKB_KEY_parenleft, Key_ParenLeft },
                XkbToKeyEntry{ XKB_KEY_parenright, Key_ParenRight },
                XkbToKeyEntry{ XKB_KEY_asterisk, Key_Asterisk },
                XkbToKeyEntry{ XKB_KEY_plus, Key_Plus },
                XkbToKeyEntry{ XKB_KEY_comma, Key_Comma },
                XkbToKeyEntry{ XKB_KEY_minus, Key_Minus },
                XkbToKeyEntry{ XKB_KEY_period, Key_Period },
                XkbToKeyEntry{ XKB_KEY_slash, Key_Slash },
                XkbToKeyEntry{ XKB_KEY_0, Key_0 },
                XkbToKeyEntry{ XKB_KEY_1, Key_1 },
                XkbToKeyEntry{ XKB_KEY_2, Key_2 },
                XkbToKeyEntry{ XKB_KEY_3, Key_3 },
                XkbToKeyEntry{ XKB_KEY_4, Key_4 },
                XkbToKeyEntry{ XKB_KEY_5, Key_5 },
                XkbToKeyEntry{ XKB_KEY_6, Key_6 },
                XkbToKeyEntry{ XKB_KEY_7, Key_7 },
                XkbToKeyEntry{ XKB_KEY_8, Key_8 },
                XkbToKeyEntry{ XKB_KEY_9, Key_9 },
                XkbToKeyEntry{ XKB_KEY_colon, Key_Colon },
                XkbToKeyEntry{ XKB_KEY_semicolon, Key_Semicolon },
                XkbToKeyEntry{ XKB_KEY_less, Key_Less },
                XkbToKeyEntry{ XKB_KEY_equal, Key_Equal },
                XkbToKeyEntry{ XKB_KEY_greater, Key_Greater },
                XkbToKeyEntry{ XKB_KEY_question, Key_Question },
                XkbToKeyEntry{ XKB_KEY_at, Key_At },
                XkbToKeyEntry{ XKB_KEY_A, Key_A },
                XkbToKeyEntry{ XKB_KEY_B, Key_B },
                XkbToKeyEntry{ XKB_KEY_C, Key_C },
                XkbToKeyEntry{ XKB_KEY_D, Key_D },
                XkbToKeyEntry{ XKB_KEY_E, Key_E },
                XkbToKeyEntry{ XKB_KEY_F, Key_F },
                XkbToKeyEntry{ XKB_KEY_G, Key_G },
                XkbToKeyEntry{ XKB_KEY_H, Key_H },
                XkbToKeyEntry{ XKB_KEY_I, Key_I },
                XkbToKeyEntry{ XKB_KEY_J, Key_J },
                XkbToKeyEntry{ XKB_KEY_K, Key_K },
                XkbToKeyEntry{ XKB_KEY_L, Key_L },
                XkbToKeyEntry{ XKB_KEY_M, Key_M },
                XkbToKeyEntry{ XKB_KEY_N, Key_N },
                XkbToKeyEntry{ XKB_KEY_O, Key_O },
                XkbToKeyEntry{ XKB_KEY_P, Key_P },
                XkbToKeyEntry{ XKB_KEY_Q, Key_Q },
                XkbToKeyEntry{ XKB_KEY_R, Key_R },
                XkbToKeyEntry{ XKB_KEY_S, Key_S },
                XkbToKeyEntry{ XKB_KEY_T, Key_T },
                XkbToKeyEntry{ XKB_KEY_U, Key_U },
                XkbToKeyEntry{ XKB_KEY_V, Key_V },
                XkbToKeyEntry{ XKB_KEY_W, Key_W },
                XkbToKeyEntry{ XKB_KEY_X, Key_X },
                XkbToKeyEntry{ XKB_KEY_Y, Key_Y },
                XkbToKeyEntry{ XKB_KEY_Z, Key_Z },
                XkbToKeyEntry{ XKB_KEY_a, Key_A },
                XkbToKeyEntry{ XKB_KEY_b, Key_B },
                XkbToKeyEntry{ XKB_KEY_c, Key_C },
                XkbToKeyEntry{ XKB_KEY_d, Key_D },
                XkbToKeyEntry{ XKB_KEY_e, Key_E },
                XkbToKeyEntry{ XKB_KEY_f, Key_F },
                XkbToKeyEntry{ XKB_KEY_g, Key_G },
                XkbToKeyEntry{ XKB_KEY_h, Key_H },
                XkbToKeyEntry{ XKB_KEY_i, Key_I },
                XkbToKeyEntry{ XKB_KEY_j, Key_J },
                XkbToKeyEntry{ XKB_KEY_k, Key_K },
                XkbToKeyEntry{ XKB_KEY_l, Key_L },
                XkbToKeyEntry{ XKB_KEY_m, Key_M },
                XkbToKeyEntry{ XKB_KEY_n, Key_N },
                XkbToKeyEntry{ XKB_KEY_o, Key_O },
                XkbToKeyEntry{ XKB_KEY_p, Key_P },
                XkbToKeyEntry{ XKB_KEY_q, Key_Q },
                XkbToKeyEntry{ XKB_KEY_r, Key_R },
                XkbToKeyEntry{ XKB_KEY_s, Key_S },
                XkbToKeyEntry{ XKB_KEY_t, Key_T },
                XkbToKeyEntry{ XKB_KEY_u, Key_U },
                XkbToKeyEntry{ XKB_KEY_v, Key_V },
                XkbToKeyEntry{ XKB_KEY_w, Key_W },
                XkbToKeyEntry{ XKB_KEY_x, Key_X },
                XkbToKeyEntry{ XKB_KEY_y, Key_Y },
                XkbToKeyEntry{ XKB_KEY_z, Key_Z },
                XkbToKeyEntry{ XKB_KEY_bracketleft, Key_BracketLeft },
                XkbToKeyEntry{ XKB_KEY_backslash, Key_Backslash },
                XkbToKeyEntry{ XKB_KEY_bracketright, Key_BracketRight },
                XkbToKeyEntry{ XKB_KEY_asciicircum, Key_AsciiCircum },
                XkbToKeyEntry{ XKB_KEY_underscore, Key_Underscore },
                XkbToKeyEntry{ XKB_KEY_quoteleft, Key_QuoteLeft },
                XkbToKeyEntry{ XKB_KEY_braceleft, Key_BraceLeft },
                XkbToKeyEntry{ XKB_KEY_bar, Key_Bar },
                XkbToKeyEntry{ XKB_KEY_braceright, Key_BraceRight },
                XkbToKeyEntry{ XKB_KEY_asciitilde, Key_AsciiTilde },

                // Function Keys, numpad and others
                XkbToKeyEntry{ XKB_KEY_Escape, Key_Escape },
                XkbToKeyEntry{ XKB_KEY_Return, Key_Enter },
                XkbToKeyEntry{ XKB_KEY_Tab, Key_Tab },
                XkbToKeyEntry{ XKB_KEY_BackSpace, Key_Backspace },
                XkbToKeyEntry{ XKB_KEY_Insert, Key_Insert },
                XkbToKeyEntry{ XKB_KEY_Delete, Key_Delete },
                XkbToKeyEntry{ XKB_KEY_Right, Key_Right },
                XkbToKeyEntry{ XKB_KEY_Left, Key_Left },
                XkbToKeyEntry{ XKB_KEY_Down, Key_Down },
                XkbToKeyEntry{ XKB_KEY_Up, Key_Up },
                XkbToKeyEntry{ XKB_KEY_Page_Up, Key_PageUp },
                XkbToKeyEntry{ XKB_KEY_Page_Down, Key_PageDown },
                XkbToKeyEntry{ XKB_KEY_Home, Key_Home },
                XkbToKeyEntry{ XKB_KEY_End, Key_End },
                XkbToKeyEntry{ XKB_KEY_Caps_Lock, Key_CapsLock },
                XkbToKeyEntry{ XKB_KEY_Scroll_Lock, Key_ScrollLock },
                XkbToKeyEntry{ XKB_KEY_Num_Lock, Key_NumLock },
                XkbToKeyEntry{ XKB_KEY_Print, Key_PrintScreen },
                XkbToKeyEntry{ XKB_KEY_Pause, Key_Pause },

                XkbToKeyEntry{ XKB_KEY_F1, Key_F1 },
                XkbToKeyEntry{ XKB_KEY_F2, Key_F2 },
                XkbToKeyEntry{ XKB_KEY_F3, Key_F3 },
                XkbToKeyEntry{ XKB_KEY_F4, Key_F4 },
                XkbToKeyEntry{ XKB_KEY_F5, Key_F5 },
                XkbToKeyEntry{ XKB_KEY_F6, Key_F6 },
                XkbToKeyEntry{ XKB_KEY_F7, Key_F7 },
                XkbToKeyEntry{ XKB_KEY_F8, Key_F8 },
                XkbToKeyEntry{ XKB_KEY_F9, Key_F9 },
                XkbToKeyEntry{ XKB_KEY_F10, Key_F10 },
                XkbToKeyEntry{ XKB_KEY_F11, Key_F11 },
                XkbToKeyEntry{ XKB_KEY_F12, Key_F12 },
                XkbToKeyEntry{ XKB_KEY_F13, Key_F13 },
                XkbToKeyEntry{ XKB_KEY_F14, Key_F14 },
                XkbToKeyEntry{ XKB_KEY_F15, Key_F15 },
                XkbToKeyEntry{ XKB_KEY_F16, Key_F16 },
                XkbToKeyEntry{ XKB_KEY_F17, Key_F17 },
                XkbToKeyEntry{ XKB_KEY_F18, Key_F18 },
                XkbToKeyEntry{ XKB_KEY_F19, Key_F19 },
                XkbToKeyEntry{ XKB_KEY_F20, Key_F20 },
                XkbToKeyEntry{ XKB_KEY_F21, Key_F21 },
                XkbToKeyEntry{ XKB_KEY_F22, Key_F22 },
                XkbToKeyEntry{ XKB_KEY_F23, Key_F23 },
                XkbToKeyEntry{ XKB_KEY_F24, Key_F24 },
                XkbToKeyEntry{ XKB_KEY_F25, Key_F25 },
                XkbToKeyEntry{ XKB_KEY_F26, Key_F26 },
                XkbToKeyEntry{ XKB_KEY_F27, Key_F27 },
                XkbToKeyEntry{ XKB_KEY_F28, Key_F28 },
                XkbToKeyEntry{ XKB_KEY_F29, Key_F29 },
                XkbToKeyEntry{ XKB_KEY_F30, Key_F30 },

                XkbToKeyEntry{ XKB_KEY_KP_0, Key_NumPad_0 },
                XkbToKeyEntry{ XKB_KEY_KP_1, Key_NumPad_1 },
                XkbToKeyEntry{ XKB_KEY_KP_2, Key_NumPad_2 },
                XkbToKeyEntry{ XKB_KEY_KP_3, Key_NumPad_3 },
                XkbToKeyEntry{ XKB_KEY_KP_4, Key_NumPad_4 },
                XkbToKeyEntry{ XKB_KEY_KP_5, Key_NumPad_5 },
                XkbToKeyEntry{ XKB_KEY_KP_6, Key_NumPad_6 },
                XkbToKeyEntry{ XKB_KEY_KP_7, Key_NumPad_7 },
                XkbToKeyEntry{ XKB_KEY_KP_8, Key_NumPad_8 },
                XkbToKeyEntry{ XKB_KEY_KP_9, Key_NumPad_9 },
                XkbToKeyEntry{ XKB_KEY_KP_Decimal, Key_NumPad_Decimal },
                XkbToKeyEntry{ XKB_KEY_KP_Divide, Key_NumPad_Divide },
                XkbToKeyEntry{ XKB_KEY_KP_Multiply, Key_NumPad_Multiply },
                XkbToKeyEntry{ XKB_KEY_KP_Subtract, Key_NumPad_Subtract },
                XkbToKeyEntry{ XKB_KEY_KP_Add, Key_NumPad_Add },
                XkbToKeyEntry{ XKB_KEY_KP_Enter, Key_NumPad_Enter },
                XkbToKeyEntry{ XKB_KEY_KP_Equal, Key_NumPad_Equal },

                XkbToKeyEntry{ XKB_KEY_Shift_L, Key_LeftShift },
                XkbToKeyEntry{ XKB_KEY_Control_L, Key_LeftControl },
                XkbToKeyEntry{ XKB_KEY_Alt_L, Key_LeftAlt },
                XkbToKeyEntry{ XKB_KEY_Super_L, Key_LeftSuper },
                XkbToKeyEntry{ XKB_KEY_Shift_R, Key_RightShift },
                XkbToKeyEntry{ XKB_KEY_Control_R, Key_RightControl },
                XkbToKeyEntry{ XKB_KEY_Alt_R, Key_RightAlt },
                XkbToKeyEntry{ XKB_KEY_Super_R, Key_RightSuper },
                XkbToKeyEntry{ XKB_KEY_Menu, Key_Menu } });

KDGui::Key keysymToKey(xkb_keysym_t keysym)
{
    const XkbToKeyEntry needle{ keysym, Key_Unknown };
    const auto it = std::lower_bound(keysymTable.cbegin(), keysymTable.cend(), needle);
    if (it != keysymTable.cend() && needle.keysym == it->keysym)
        return it->key;
    return Key_Unknown;
}

} // namespace KDGui::xkb
