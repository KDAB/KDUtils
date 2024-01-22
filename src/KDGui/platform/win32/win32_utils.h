/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <windows.h>

#include <string>
#include <string_view>

namespace KDGui {

std::string windowsErrorMessage(unsigned long errorCode);

std::wstring utf8StringToWide(std::string_view source);
std::string wideStringToUtf8(std::wstring_view source);
} // namespace KDGui
