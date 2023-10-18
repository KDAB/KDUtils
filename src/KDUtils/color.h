/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cstdint>
#include <climits>
#include <string>
#include <cassert>
#include <iostream>

namespace KDUtils {

constexpr uint8_t hexToInt(const char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else
        throw std::runtime_error("Invalid hex code");
}

template<typename V, typename C = float>
constexpr V hexToRgb(const char *hex)
{
    if (hex[0] != '#')
        throw std::runtime_error("Missing hashtag at start of hexadecimal string");

    if (std::char_traits<char>::length(hex) != 7)
        throw std::runtime_error("Length of hexadecimal string must be 7 characters");

    if constexpr (std::is_integral_v<C>) {
        // Do not normalize integral type components
        return V{
            static_cast<C>((hexToInt(hex[1]) << 4 | hexToInt(hex[2])) & 0xff),
            static_cast<C>((hexToInt(hex[3]) << 4 | hexToInt(hex[4])) & 0xff),
            static_cast<C>((hexToInt(hex[5]) << 4 | hexToInt(hex[6])) & 0xff)
        };
    } else {
        // Normalize floating point type components
        return V{
            static_cast<C>((hexToInt(hex[1]) << 4 | hexToInt(hex[2])) & 0xff) / C(255.0),
            static_cast<C>((hexToInt(hex[3]) << 4 | hexToInt(hex[4])) & 0xff) / C(255.0),
            static_cast<C>((hexToInt(hex[5]) << 4 | hexToInt(hex[6])) & 0xff) / C(255.0)
        };
    }
}

template<typename V, typename C = float>
constexpr V hexToRgba(const char *hex, C alpha)
{
    if (hex[0] != '#')
        throw std::runtime_error("Missing hashtag at start of hexadecimal string");

    if (std::char_traits<char>::length(hex) != 7)
        throw std::runtime_error("Length of hexadecimal string must be 7 characters");

    if constexpr (std::is_integral_v<C>) {
        // Do not normalize integral type components
        return V{
            static_cast<C>((hexToInt(hex[1]) << 4 | hexToInt(hex[2])) & 0xff),
            static_cast<C>((hexToInt(hex[3]) << 4 | hexToInt(hex[4])) & 0xff),
            static_cast<C>((hexToInt(hex[5]) << 4 | hexToInt(hex[6])) & 0xff),
            alpha
        };
    } else {
        // Normalize floating point type components
        return V{
            static_cast<C>((hexToInt(hex[1]) << 4 | hexToInt(hex[2])) & 0xff) / C(255.0),
            static_cast<C>((hexToInt(hex[3]) << 4 | hexToInt(hex[4])) & 0xff) / C(255.0),
            static_cast<C>((hexToInt(hex[5]) << 4 | hexToInt(hex[6])) & 0xff) / C(255.0),
            alpha
        };
    }
}
} // namespace KDUtils
