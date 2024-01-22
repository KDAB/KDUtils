/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace KDFoundation {

struct Extent2D {
    uint32_t width{ 0 };
    uint32_t height{ 0 };

    bool operator==(const Extent2D &other) const noexcept
    {
        return width == other.width && height == other.height;
    }
};

struct Position2D {
    int32_t x{ 0 };
    int32_t y{ 0 };

    bool operator==(const Position2D &other) const noexcept
    {
        return x == other.x && y == other.y;
    }
};

struct Rect2D {
    Position2D position;
    Extent2D extent;

    bool operator==(const Rect2D &other) const noexcept
    {
        return position == other.position && extent == other.extent;
    }

    bool isNull() const { return extent.width == 0 || extent.height == 0; }
};

[[nodiscard]] inline bool fuzzyCompare(double p1, double p2)
{
    return (std::abs(p1 - p2) * 1000000000000. <= std::min(std::abs(p1), std::abs(p2)));
}

[[nodiscard]] inline bool fuzzyCompare(float p1, float p2)
{
    return (std::abs(p1 - p2) * 100000.f <= std::min(std::abs(p1), std::abs(p2)));
}

[[nodiscard]] inline bool fuzzyCompare(glm::vec2 p1, glm::vec2 p2)
{
    return fuzzyCompare(p1.x, p2.x) && fuzzyCompare(p1.y, p2.y);
}

[[nodiscard]] inline bool fuzzyCompare(glm::vec3 p1, glm::vec3 p2)
{
    return fuzzyCompare(p1.x, p2.x) && fuzzyCompare(p1.y, p2.y) && fuzzyCompare(p1.z, p2.z);
}

[[nodiscard]] inline bool fuzzyCompare(glm::vec4 p1, glm::vec4 p2)
{
    return fuzzyCompare(p1.x, p2.x) && fuzzyCompare(p1.y, p2.y) && fuzzyCompare(p1.z, p2.z) && fuzzyCompare(p1.w, p2.w);
}

[[nodiscard]] inline bool fuzzyCompare(glm::quat p1, glm::quat p2)
{
    return fuzzyCompare(p1.x, p2.x) && fuzzyCompare(p1.y, p2.y) && fuzzyCompare(p1.z, p2.z) && fuzzyCompare(p1.w, p2.w);
}

[[nodiscard]] inline bool fuzzyIsNull(double d)
{
    return std::abs(d) <= 0.000000000001;
}

[[nodiscard]] inline bool fuzzyIsNull(float f)
{
    return std::abs(f) <= 0.00001f;
}

} // namespace KDFoundation
