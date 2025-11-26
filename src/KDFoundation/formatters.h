/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

template<>
struct KDFOUNDATION_API fmt::formatter<glm::vec3> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(glm::vec3 const &v, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "Vec3({:f}, {:f}, {:f})", v[0], v[1], v[2]);
    }
};

template<>
struct KDFOUNDATION_API fmt::formatter<glm::vec4> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(glm::vec4 const &v, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "Vec4({:f}, {:f}, {:f} {:f})", v[0], v[1], v[2], v[3]);
    }
};

template<>
struct KDFOUNDATION_API fmt::formatter<glm::quat> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(glm::quat const &v, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "Quat({:f}, {:f}, {:f} {:f})", v[0], v[1], v[2], v[3]);
    }
};

template<>
struct KDFOUNDATION_API fmt::formatter<glm::mat4> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(glm::mat4 const &v, FormatContext &ctx) const
    {
        return fmt::format_to(ctx.out(), "Mat4({:f}, {:f}, {:f} {:f}\n     {:f}, {:f}, {:f} {:f}\n     {:f}, {:f}, {:f} {:f}\n     {:f}, {:f}, {:f} {:f})",
                              v[0][0], v[1][0], v[2][0], v[3][0],
                              v[0][1], v[1][1], v[2][1], v[3][1],
                              v[0][2], v[1][2], v[2][2], v[3][2],
                              v[0][3], v[1][3], v[2][3], v[3][3]);
    }
};
