/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Alstair Baxter <alistair.baxter@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_keyboard_map.h"

#include <android/input.h>
#include <android/keycodes.h>

#include <vector>

namespace KDGui {

namespace {

const std::vector<Key> createKeyCodeTable()
{
    std::vector<Key> keyCodes;
    keyCodes.resize(512, Key_Unknown);

    // keyCodes[AKEYCODE_UNKNOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_SOFT_LEFT] = Key_Unknown;
    // keyCodes[AKEYCODE_SOFT_RIGHT] = Key_Unknown;
    // keyCodes[AKEYCODE_HOME] = Key_Unknown;
    keyCodes[AKEYCODE_BACK] = Key_Escape;
    // keyCodes[AKEYCODE_CALL] = Key_Unknown;
    // keyCodes[AKEYCODE_ENDCALL] = Key_Unknown;
    keyCodes[AKEYCODE_0] = Key_0;
    keyCodes[AKEYCODE_1] = Key_1;
    keyCodes[AKEYCODE_2] = Key_2;
    keyCodes[AKEYCODE_3] = Key_3;
    keyCodes[AKEYCODE_4] = Key_4;
    keyCodes[AKEYCODE_5] = Key_5;
    keyCodes[AKEYCODE_6] = Key_6;
    keyCodes[AKEYCODE_7] = Key_7;
    keyCodes[AKEYCODE_8] = Key_8;
    keyCodes[AKEYCODE_9] = Key_9;
    keyCodes[AKEYCODE_STAR] = Key_Asterisk;
    keyCodes[AKEYCODE_POUND] = Key_HashSign;
    keyCodes[AKEYCODE_DPAD_UP] = Key_Up;
    keyCodes[AKEYCODE_DPAD_DOWN] = Key_Down;
    keyCodes[AKEYCODE_DPAD_LEFT] = Key_Left;
    keyCodes[AKEYCODE_DPAD_RIGHT] = Key_Right;
    // keyCodes[AKEYCODE_DPAD_CENTER] = Key_Unknown;
    // keyCodes[AKEYCODE_VOLUME_UP] = Key_Unknown;
    // keyCodes[AKEYCODE_VOLUME_DOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_POWER] = Key_Unknown;
    // keyCodes[AKEYCODE_CAMERA] = Key_Unknown;
    // keyCodes[AKEYCODE_CLEAR] = Key_Unknown;
    keyCodes[AKEYCODE_A] = Key_A;
    keyCodes[AKEYCODE_B] = Key_B;
    keyCodes[AKEYCODE_C] = Key_C;
    keyCodes[AKEYCODE_D] = Key_D;
    keyCodes[AKEYCODE_E] = Key_E;
    keyCodes[AKEYCODE_F] = Key_F;
    keyCodes[AKEYCODE_G] = Key_G;
    keyCodes[AKEYCODE_H] = Key_H;
    keyCodes[AKEYCODE_I] = Key_I;
    keyCodes[AKEYCODE_J] = Key_J;
    keyCodes[AKEYCODE_K] = Key_K;
    keyCodes[AKEYCODE_L] = Key_L;
    keyCodes[AKEYCODE_M] = Key_M;
    keyCodes[AKEYCODE_N] = Key_N;
    keyCodes[AKEYCODE_O] = Key_O;
    keyCodes[AKEYCODE_P] = Key_P;
    keyCodes[AKEYCODE_Q] = Key_Q;
    keyCodes[AKEYCODE_R] = Key_R;
    keyCodes[AKEYCODE_S] = Key_S;
    keyCodes[AKEYCODE_T] = Key_T;
    keyCodes[AKEYCODE_U] = Key_U;
    keyCodes[AKEYCODE_V] = Key_V;
    keyCodes[AKEYCODE_W] = Key_W;
    keyCodes[AKEYCODE_X] = Key_X;
    keyCodes[AKEYCODE_Y] = Key_Y;
    keyCodes[AKEYCODE_Z] = Key_Z;
    keyCodes[AKEYCODE_COMMA] = Key_Comma;
    keyCodes[AKEYCODE_PERIOD] = Key_Period;
    keyCodes[AKEYCODE_ALT_LEFT] = Key_LeftAlt;
    keyCodes[AKEYCODE_ALT_RIGHT] = Key_RightAlt;
    keyCodes[AKEYCODE_SHIFT_LEFT] = Key_LeftShift;
    keyCodes[AKEYCODE_SHIFT_RIGHT] = Key_RightShift;
    keyCodes[AKEYCODE_TAB] = Key_Tab;
    keyCodes[AKEYCODE_SPACE] = Key_Space;
    // keyCodes[AKEYCODE_SYM] = Key_Unknown;
    // keyCodes[AKEYCODE_EXPLORER] = Key_Unknown;
    // keyCodes[AKEYCODE_ENVELOPE] = Key_Unknown;
    keyCodes[AKEYCODE_ENTER] = Key_Enter;
    keyCodes[AKEYCODE_DEL] = Key_Delete;
    keyCodes[AKEYCODE_GRAVE] = Key_AsciiTilde;
    keyCodes[AKEYCODE_MINUS] = Key_Minus;
    keyCodes[AKEYCODE_EQUALS] = Key_Equal;
    keyCodes[AKEYCODE_LEFT_BRACKET] = Key_BracketLeft;
    keyCodes[AKEYCODE_RIGHT_BRACKET] = Key_BracketRight;
    keyCodes[AKEYCODE_BACKSLASH] = Key_Backslash;
    keyCodes[AKEYCODE_SEMICOLON] = Key_Semicolon;
    keyCodes[AKEYCODE_APOSTROPHE] = Key_Apostrophe;
    keyCodes[AKEYCODE_SLASH] = Key_Slash;
    keyCodes[AKEYCODE_AT] = Key_At;
    keyCodes[AKEYCODE_NUM] = Key_NumLock;
    // keyCodes[AKEYCODE_HEADSETHOOK] = Key_Unknown;
    // keyCodes[AKEYCODE_FOCUS] = Key_Unknown;
    keyCodes[AKEYCODE_PLUS] = Key_Plus;
    keyCodes[AKEYCODE_MENU] = Key_Menu;
    // keyCodes[AKEYCODE_NOTIFICATION] = Key_Unknown;
    // keyCodes[AKEYCODE_SEARCH] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_PLAY_PAUSE] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_STOP] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_NEXT] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_PREVIOUS] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_REWIND] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_FAST_FORWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_MUTE] = Key_Unknown;
    keyCodes[AKEYCODE_PAGE_UP] = Key_PageUp;
    keyCodes[AKEYCODE_PAGE_DOWN] = Key_PageDown;
    // keyCodes[AKEYCODE_PICTSYMBOLS] = Key_Unknown;
    // keyCodes[AKEYCODE_SWITCH_CHARSET] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_A] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_B] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_C] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_X] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_Y] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_Z] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_L1] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_R1] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_L2] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_R2] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_THUMBL] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_THUMBR] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_START] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_SELECT] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_MODE] = Key_Unknown;
    keyCodes[AKEYCODE_ESCAPE] = Key_Escape;
    keyCodes[AKEYCODE_FORWARD_DEL] = Key_Backspace;
    keyCodes[AKEYCODE_CTRL_LEFT] = Key_LeftControl;
    keyCodes[AKEYCODE_CTRL_RIGHT] = Key_RightControl;
    keyCodes[AKEYCODE_CAPS_LOCK] = Key_CapsLock;
    keyCodes[AKEYCODE_SCROLL_LOCK] = Key_ScrollLock;
    keyCodes[AKEYCODE_META_LEFT] = Key_LeftSuper;
    keyCodes[AKEYCODE_META_RIGHT] = Key_RightSuper;
    // keyCodes[AKEYCODE_FUNCTION] = Key_Unknown;
    // keyCodes[AKEYCODE_SYSRQ] = Key_Unknown;
    // keyCodes[AKEYCODE_BREAK] = Key_Unknown;
    keyCodes[AKEYCODE_MOVE_HOME] = Key_Home;
    keyCodes[AKEYCODE_MOVE_END] = Key_End;
    keyCodes[AKEYCODE_INSERT] = Key_Insert;
    // keyCodes[AKEYCODE_FORWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_PLAY] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_PAUSE] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_CLOSE] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_EJECT] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_RECORD] = Key_Unknown;
    keyCodes[AKEYCODE_F1] = Key_F1;
    keyCodes[AKEYCODE_F2] = Key_F2;
    keyCodes[AKEYCODE_F3] = Key_F3;
    keyCodes[AKEYCODE_F4] = Key_F4;
    keyCodes[AKEYCODE_F5] = Key_F5;
    keyCodes[AKEYCODE_F6] = Key_F6;
    keyCodes[AKEYCODE_F7] = Key_F7;
    keyCodes[AKEYCODE_F8] = Key_F8;
    keyCodes[AKEYCODE_F9] = Key_F9;
    keyCodes[AKEYCODE_F10] = Key_F10;
    keyCodes[AKEYCODE_F11] = Key_F11;
    keyCodes[AKEYCODE_F12] = Key_F12;
    keyCodes[AKEYCODE_NUM_LOCK] = Key_NumLock;
    keyCodes[AKEYCODE_NUMPAD_0] = Key_NumPad_0;
    keyCodes[AKEYCODE_NUMPAD_1] = Key_NumPad_1;
    keyCodes[AKEYCODE_NUMPAD_2] = Key_NumPad_2;
    keyCodes[AKEYCODE_NUMPAD_3] = Key_NumPad_3;
    keyCodes[AKEYCODE_NUMPAD_4] = Key_NumPad_4;
    keyCodes[AKEYCODE_NUMPAD_5] = Key_NumPad_5;
    keyCodes[AKEYCODE_NUMPAD_6] = Key_NumPad_6;
    keyCodes[AKEYCODE_NUMPAD_7] = Key_NumPad_7;
    keyCodes[AKEYCODE_NUMPAD_8] = Key_NumPad_8;
    keyCodes[AKEYCODE_NUMPAD_9] = Key_NumPad_9;
    keyCodes[AKEYCODE_NUMPAD_DIVIDE] = Key_NumPad_Divide;
    keyCodes[AKEYCODE_NUMPAD_MULTIPLY] = Key_NumPad_Multiply;
    keyCodes[AKEYCODE_NUMPAD_SUBTRACT] = Key_NumPad_Subtract;
    keyCodes[AKEYCODE_NUMPAD_ADD] = Key_NumPad_Add;
    keyCodes[AKEYCODE_NUMPAD_DOT] = Key_NumPad_Decimal;
    // keyCodes[AKEYCODE_NUMPAD_COMMA] = Key_Unknown;
    keyCodes[AKEYCODE_NUMPAD_ENTER] = Key_NumPad_Enter;
    keyCodes[AKEYCODE_NUMPAD_EQUALS] = Key_NumPad_Equal;
    // keyCodes[AKEYCODE_NUMPAD_LEFT_PAREN] = Key_Unknown;
    // keyCodes[AKEYCODE_NUMPAD_RIGHT_PAREN] = Key_Unknown;
    // keyCodes[AKEYCODE_VOLUME_MUTE] = Key_Unknown;
    // keyCodes[AKEYCODE_INFO] = Key_Unknown;
    // keyCodes[AKEYCODE_CHANNEL_UP] = Key_Unknown;
    // keyCodes[AKEYCODE_CHANNEL_DOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_ZOOM_IN] = Key_Unknown;
    // keyCodes[AKEYCODE_ZOOM_OUT] = Key_Unknown;
    // keyCodes[AKEYCODE_TV] = Key_Unknown;
    // keyCodes[AKEYCODE_WINDOW] = Key_Unknown;
    // keyCodes[AKEYCODE_GUIDE] = Key_Unknown;
    // keyCodes[AKEYCODE_DVR] = Key_Unknown;
    // keyCodes[AKEYCODE_BOOKMARK] = Key_Unknown;
    // keyCodes[AKEYCODE_CAPTIONS] = Key_Unknown;
    // keyCodes[AKEYCODE_SETTINGS] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_POWER] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT] = Key_Unknown;
    // keyCodes[AKEYCODE_STB_POWER] = Key_Unknown;
    // keyCodes[AKEYCODE_STB_INPUT] = Key_Unknown;
    // keyCodes[AKEYCODE_AVR_POWER] = Key_Unknown;
    // keyCodes[AKEYCODE_AVR_INPUT] = Key_Unknown;
    // keyCodes[AKEYCODE_PROG_RED] = Key_Unknown;
    // keyCodes[AKEYCODE_PROG_GREEN] = Key_Unknown;
    // keyCodes[AKEYCODE_PROG_YELLOW] = Key_Unknown;
    // keyCodes[AKEYCODE_PROG_BLUE] = Key_Unknown;
    // keyCodes[AKEYCODE_APP_SWITCH] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_1] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_2] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_3] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_4] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_5] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_6] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_7] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_8] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_9] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_10] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_11] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_12] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_13] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_14] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_15] = Key_Unknown;
    // keyCodes[AKEYCODE_BUTTON_16] = Key_Unknown;
    // keyCodes[AKEYCODE_LANGUAGE_SWITCH] = Key_Unknown;
    // keyCodes[AKEYCODE_MANNER_MODE] = Key_Unknown;
    // keyCodes[AKEYCODE_3D_MODE] = Key_Unknown;
    // keyCodes[AKEYCODE_CONTACTS] = Key_Unknown;
    // keyCodes[AKEYCODE_CALENDAR] = Key_Unknown;
    // keyCodes[AKEYCODE_MUSIC] = Key_Unknown;
    // keyCodes[AKEYCODE_CALCULATOR] = Key_Unknown;
    // keyCodes[AKEYCODE_ZENKAKU_HANKAKU] = Key_Unknown;
    // keyCodes[AKEYCODE_EISU] = Key_Unknown;
    // keyCodes[AKEYCODE_MUHENKAN] = Key_Unknown;
    // keyCodes[AKEYCODE_HENKAN] = Key_Unknown;
    // keyCodes[AKEYCODE_KATAKANA_HIRAGANA] = Key_Unknown;
    // keyCodes[AKEYCODE_YEN] = Key_Unknown;
    // keyCodes[AKEYCODE_RO] = Key_Unknown;
    // keyCodes[AKEYCODE_KANA] = Key_Unknown;
    // keyCodes[AKEYCODE_ASSIST] = Key_Unknown;
    // keyCodes[AKEYCODE_BRIGHTNESS_DOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_BRIGHTNESS_UP] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_AUDIO_TRACK] = Key_Unknown;
    // keyCodes[AKEYCODE_SLEEP] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_RADIO_SERVICE] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_TELETEXT] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_NUMBER_ENTRY] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_TERRESTRIAL_ANALOG] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_TERRESTRIAL_DIGITAL] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_SATELLITE] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_SATELLITE_BS] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_SATELLITE_CS] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_SATELLITE_SERVICE] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_NETWORK] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_ANTENNA_CABLE] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_HDMI_1] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_HDMI_2] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_HDMI_3] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_HDMI_4] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_COMPOSITE_1] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_COMPOSITE_2] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_COMPONENT_1] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_COMPONENT_2] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_INPUT_VGA_1] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_AUDIO_DESCRIPTION] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_ZOOM_MODE] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_CONTENTS_MENU] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_MEDIA_CONTEXT_MENU] = Key_Unknown;
    // keyCodes[AKEYCODE_TV_TIMER_PROGRAMMING] = Key_Unknown;
    // keyCodes[AKEYCODE_HELP] = Key_Unknown;
    // keyCodes[AKEYCODE_NAVIGATE_PREVIOUS] = Key_Unknown;
    // keyCodes[AKEYCODE_NAVIGATE_NEXT] = Key_Unknown;
    // keyCodes[AKEYCODE_NAVIGATE_IN] = Key_Unknown;
    // keyCodes[AKEYCODE_NAVIGATE_OUT] = Key_Unknown;
    // keyCodes[AKEYCODE_STEM_PRIMARY] = Key_Unknown;
    // keyCodes[AKEYCODE_STEM_1] = Key_Unknown;
    // keyCodes[AKEYCODE_STEM_2] = Key_Unknown;
    // keyCodes[AKEYCODE_STEM_3] = Key_Unknown;
    // keyCodes[AKEYCODE_DPAD_UP_LEFT] = Key_Unknown;
    // keyCodes[AKEYCODE_DPAD_DOWN_LEFT] = Key_Unknown;
    // keyCodes[AKEYCODE_DPAD_UP_RIGHT] = Key_Unknown;
    // keyCodes[AKEYCODE_DPAD_DOWN_RIGHT] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_SKIP_FORWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_SKIP_BACKWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_STEP_FORWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_MEDIA_STEP_BACKWARD] = Key_Unknown;
    // keyCodes[AKEYCODE_SOFT_SLEEP] = Key_Unknown;
    // keyCodes[AKEYCODE_CUT] = Key_Unknown;
    // keyCodes[AKEYCODE_COPY] = Key_Unknown;
    // keyCodes[AKEYCODE_PASTE] = Key_Unknown;
    keyCodes[AKEYCODE_SYSTEM_NAVIGATION_UP] = Key_Up;
    keyCodes[AKEYCODE_SYSTEM_NAVIGATION_DOWN] = Key_Down;
    keyCodes[AKEYCODE_SYSTEM_NAVIGATION_LEFT] = Key_Left;
    keyCodes[AKEYCODE_SYSTEM_NAVIGATION_RIGHT] = Key_Right;
    // keyCodes[AKEYCODE_ALL_APPS] = Key_Unknown;
    // keyCodes[AKEYCODE_REFRESH] = Key_Unknown;
    // keyCodes[AKEYCODE_THUMBS_UP] = Key_Unknown;
    // keyCodes[AKEYCODE_THUMBS_DOWN] = Key_Unknown;
    // keyCodes[AKEYCODE_PROFILE_SWITCH] = Key_Unknown;

    return keyCodes;
}
} // namespace

Key androidKeyCodeToKey(unsigned keyCode)
{
    static const auto keyCodeTable = createKeyCodeTable();

    if (keyCode >= keyCodeTable.size())
        return Key_Unknown;
    return keyCodeTable[keyCode];
}

KeyboardModifiers androidKeyMetaToModifiers(int32_t keyMeta)
{
    KeyboardModifiers modifiers{ Mod_NoModifiers };

    if (keyMeta & (AMETA_ALT_ON | AMETA_ALT_LEFT_ON | AMETA_ALT_RIGHT_ON))
        modifiers |= Mod_Alt;
    if (keyMeta & (AMETA_CTRL_ON | AMETA_CTRL_LEFT_ON | AMETA_CTRL_RIGHT_ON))
        modifiers |= Mod_Control;
    if (keyMeta & (AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON))
        modifiers |= Mod_Shift;
    if (keyMeta & (AMETA_META_ON | AMETA_META_LEFT_ON | AMETA_META_RIGHT_ON))
        modifiers |= Mod_Logo;
    if (keyMeta & AMETA_CAPS_LOCK_ON)
        modifiers |= Mod_CapsLock;
    if (keyMeta & AMETA_NUM_LOCK_ON)
        modifiers |= Mod_NumLock;

    return modifiers;
}

} // namespace KDGui
