/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "linux_xkb.h"

#include <xkbcommon/xkbcommon.h>

using namespace KDGui;

KeyboardModifiers xkb::modifierState(xkb_state *state)
{
    KeyboardModifiers modifiers{ Mod_NoModifiers };

    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_Shift;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_Control;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_Alt;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_Logo;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_CapsLock;
    if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_NUM, XKB_STATE_MODS_EFFECTIVE) == 1)
        modifiers |= Mod_NumLock;

    return modifiers;
}
