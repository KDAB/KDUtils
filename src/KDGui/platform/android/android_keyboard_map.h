/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Alstair Baxter <alistair.baxter@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/kdgui_keys.h>

namespace KDGui {

Key androidKeyCodeToKey(unsigned keyCode);

KeyboardModifiers androidKeyMetaToModifiers(int32_t keyMeta);

} // namespace KDGui
