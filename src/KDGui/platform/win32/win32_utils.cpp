/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "win32_utils.h"

#include <KDUtils/logging.h>

namespace KDGui {

std::string windowsErrorMessage(unsigned long errorCode)
{
    char *messageBuffer = nullptr;
    auto length = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr, errorCode, 0, reinterpret_cast<char *>(&messageBuffer), 0, nullptr);
    if (!length)
        return "Unknown error " + std::to_string(errorCode);
    auto result = std::string(messageBuffer);
    LocalFree(messageBuffer);
    return result;
}

std::wstring utf8StringToWide(std::string_view source)
{
    const auto count = MultiByteToWideChar(CP_UTF8, 0, source.data(), static_cast<int>(source.size()), nullptr, 0);
    if (!count) {
        SPDLOG_WARN("Failed to convert string from UTF-8");
        return {};
    }

    std::wstring result(count, 0);
    if (!MultiByteToWideChar(CP_UTF8, 0, source.data(), static_cast<int>(source.size()), result.data(), count)) {
        SPDLOG_WARN("Failed to convert string from UTF-8");
        return {};
    }

    return result;
}

std::string wideStringToUtf8(std::wstring_view source)
{
    const auto count = WideCharToMultiByte(CP_UTF8, 0, source.data(), static_cast<int>(source.size()), nullptr, 0, nullptr, nullptr);
    if (!count) {
        SPDLOG_WARN("Failed to convert string to UTF-8");
        return {};
    }

    std::string result(count, 0);
    if (!WideCharToMultiByte(CP_UTF8, 0, source.data(), static_cast<int>(source.size()), result.data(), count, nullptr, nullptr)) {
        SPDLOG_WARN("Failed to convert string to UTF-8");
        return {};
    }

    return result;
}

} // namespace KDGui
