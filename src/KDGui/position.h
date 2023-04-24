/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGui/kdgui_global.h>

namespace KDGui {

class KDGUI_API Position
{
public:
    int64_t x;
    int64_t y;

    Position(int64_t _x = 0, int64_t _y = 0)
        : x(_x), y(_y) { }

    bool operator==(const Position &other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position &other) const { return x != other.x && y != other.y; }
    Position &operator+=(const Position &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Position &operator/=(int64_t v)
    {
        x /= v;
        y /= v;
        return *this;
    }
    Position &operator*=(int64_t v)
    {
        x *= v;
        y *= v;
        return *this;
    }
};

inline Position operator+(const Position &a, const Position &b)
{
    return Position(a.x + b.x, a.y + b.y);
}
inline Position operator-(const Position &a, const Position &b)
{
    return Position(a.x - b.x, a.y - b.y);
}
inline Position operator/(const Position &a, int64_t v)
{
    return Position(a.x / v, a.y / v);
}

} // namespace KDGui
