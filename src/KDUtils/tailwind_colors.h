/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sean Harmer <sean.harmer@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "color.h"

#include <cstdint>
#include <type_traits>
#include <cassert>

namespace KDUtils {

enum class TailwindColor : uint16_t {
    Black,
    White,

    Slate50,
    Slate100,
    Slate200,
    Slate300,
    Slate400,
    Slate500,
    Slate600,
    Slate700,
    Slate800,
    Slate900,
    Slate950,

    Gray50,
    Gray100,
    Gray200,
    Gray300,
    Gray400,
    Gray500,
    Gray600,
    Gray700,
    Gray800,
    Gray900,
    Gray950,

    Zinc50,
    Zinc100,
    Zinc200,
    Zinc300,
    Zinc400,
    Zinc500,
    Zinc600,
    Zinc700,
    Zinc800,
    Zinc900,
    Zinc950,

    Neutral50,
    Neutral100,
    Neutral200,
    Neutral300,
    Neutral400,
    Neutral500,
    Neutral600,
    Neutral700,
    Neutral800,
    Neutral900,
    Neutral950,

    Stone50,
    Stone100,
    Stone200,
    Stone300,
    Stone400,
    Stone500,
    Stone600,
    Stone700,
    Stone800,
    Stone900,
    Stone950,

    Red50,
    Red100,
    Red200,
    Red300,
    Red400,
    Red500,
    Red600,
    Red700,
    Red800,
    Red900,
    Red950,

    Orange50,
    Orange100,
    Orange200,
    Orange300,
    Orange400,
    Orange500,
    Orange600,
    Orange700,
    Orange800,
    Orange900,
    Orange950,

    Amber50,
    Amber100,
    Amber200,
    Amber300,
    Amber400,
    Amber500,
    Amber600,
    Amber700,
    Amber800,
    Amber900,
    Amber950,

    Yellow50,
    Yellow100,
    Yellow200,
    Yellow300,
    Yellow400,
    Yellow500,
    Yellow600,
    Yellow700,
    Yellow800,
    Yellow900,
    Yellow950,

    Lime50,
    Lime100,
    Lime200,
    Lime300,
    Lime400,
    Lime500,
    Lime600,
    Lime700,
    Lime800,
    Lime900,
    Lime950,

    Green50,
    Green100,
    Green200,
    Green300,
    Green400,
    Green500,
    Green600,
    Green700,
    Green800,
    Green900,
    Green950,

    Emerald50,
    Emerald100,
    Emerald200,
    Emerald300,
    Emerald400,
    Emerald500,
    Emerald600,
    Emerald700,
    Emerald800,
    Emerald900,
    Emerald950,

    Teal50,
    Teal100,
    Teal200,
    Teal300,
    Teal400,
    Teal500,
    Teal600,
    Teal700,
    Teal800,
    Teal900,
    Teal950,

    Cyan50,
    Cyan100,
    Cyan200,
    Cyan300,
    Cyan400,
    Cyan500,
    Cyan600,
    Cyan700,
    Cyan800,
    Cyan900,
    Cyan950,

    Sky50,
    Sky100,
    Sky200,
    Sky300,
    Sky400,
    Sky500,
    Sky600,
    Sky700,
    Sky800,
    Sky900,
    Sky950,

    Blue50,
    Blue100,
    Blue200,
    Blue300,
    Blue400,
    Blue500,
    Blue600,
    Blue700,
    Blue800,
    Blue900,
    Blue950,

    Indigo50,
    Indigo100,
    Indigo200,
    Indigo300,
    Indigo400,
    Indigo500,
    Indigo600,
    Indigo700,
    Indigo800,
    Indigo900,
    Indigo950,

    Violet50,
    Violet100,
    Violet200,
    Violet300,
    Violet400,
    Violet500,
    Violet600,
    Violet700,
    Violet800,
    Violet900,
    Violet950,

    Purple50,
    Purple100,
    Purple200,
    Purple300,
    Purple400,
    Purple500,
    Purple600,
    Purple700,
    Purple800,
    Purple900,
    Purple950,

    Fuchsia50,
    Fuchsia100,
    Fuchsia200,
    Fuchsia300,
    Fuchsia400,
    Fuchsia500,
    Fuchsia600,
    Fuchsia700,
    Fuchsia800,
    Fuchsia900,
    Fuchsia950,

    Pink50,
    Pink100,
    Pink200,
    Pink300,
    Pink400,
    Pink500,
    Pink600,
    Pink700,
    Pink800,
    Pink900,
    Pink950,

    Rose50,
    Rose100,
    Rose200,
    Rose300,
    Rose400,
    Rose500,
    Rose600,
    Rose700,
    Rose800,
    Rose900,
    Rose950,
};

constexpr const char *tailwindColorToHex(TailwindColor c)
{
    // clang-format off
    switch (c) {
    case TailwindColor::Black:    return "#000000";
    case TailwindColor::White:    return "#ffffff";

    case TailwindColor::Slate50:  return "#f8fafc";
    case TailwindColor::Slate100: return "#f1f5f9";
    case TailwindColor::Slate200: return "#e2e8f0";
    case TailwindColor::Slate300: return "#cbd5e1";
    case TailwindColor::Slate400: return "#94a3b8";
    case TailwindColor::Slate500: return "#64748b";
    case TailwindColor::Slate600: return "#475569";
    case TailwindColor::Slate700: return "#334155";
    case TailwindColor::Slate800: return "#1e293b";
    case TailwindColor::Slate900: return "#0f172a";
    case TailwindColor::Slate950: return "#020617";

    case TailwindColor::Gray50:  return "#f9fafb";
    case TailwindColor::Gray100: return "#f3f4f6";
    case TailwindColor::Gray200: return "#e5e7eb";
    case TailwindColor::Gray300: return "#d1d5db";
    case TailwindColor::Gray400: return "#9ca3af";
    case TailwindColor::Gray500: return "#6b7280";
    case TailwindColor::Gray600: return "#4b5563";
    case TailwindColor::Gray700: return "#374151";
    case TailwindColor::Gray800: return "#1f2937";
    case TailwindColor::Gray900: return "#111827";
    case TailwindColor::Gray950: return "#030712";

    case TailwindColor::Zinc50:  return "#fafafa";
    case TailwindColor::Zinc100: return "#f4f4f5";
    case TailwindColor::Zinc200: return "#e4e4e7";
    case TailwindColor::Zinc300: return "#d4d4d8";
    case TailwindColor::Zinc400: return "#a1a1aa";
    case TailwindColor::Zinc500: return "#71717a";
    case TailwindColor::Zinc600: return "#52525b";
    case TailwindColor::Zinc700: return "#3f3f46";
    case TailwindColor::Zinc800: return "#27272a";
    case TailwindColor::Zinc900: return "#18181b";
    case TailwindColor::Zinc950: return "#09090b";

    case TailwindColor::Neutral50:  return "#fafafa";
    case TailwindColor::Neutral100: return "#f5f5f5";
    case TailwindColor::Neutral200: return "#e5e5e5";
    case TailwindColor::Neutral300: return "#d4d4d4";
    case TailwindColor::Neutral400: return "#a3a3a3";
    case TailwindColor::Neutral500: return "#737373";
    case TailwindColor::Neutral600: return "#525252";
    case TailwindColor::Neutral700: return "#404040";
    case TailwindColor::Neutral800: return "#262626";
    case TailwindColor::Neutral900: return "#171717";
    case TailwindColor::Neutral950: return "#0a0a0a";

    case TailwindColor::Stone50:  return "#fafaf9";
    case TailwindColor::Stone100: return "#f5f5f4";
    case TailwindColor::Stone200: return "#e7e5e4";
    case TailwindColor::Stone300: return "#d6d3d1";
    case TailwindColor::Stone400: return "#a8a29e";
    case TailwindColor::Stone500: return "#78716c";
    case TailwindColor::Stone600: return "#57534e";
    case TailwindColor::Stone700: return "#44403c";
    case TailwindColor::Stone800: return "#292524";
    case TailwindColor::Stone900: return "#1c1917";
    case TailwindColor::Stone950: return "#0c0a09";

    case TailwindColor::Red50:  return "#fef2f2";
    case TailwindColor::Red100: return "#fee2e2";
    case TailwindColor::Red200: return "#fecaca";
    case TailwindColor::Red300: return "#fca5a5";
    case TailwindColor::Red400: return "#f87171";
    case TailwindColor::Red500: return "#ef4444";
    case TailwindColor::Red600: return "#dc2626";
    case TailwindColor::Red700: return "#b91c1c";
    case TailwindColor::Red800: return "#991b1b";
    case TailwindColor::Red900: return "#7f1d1d";
    case TailwindColor::Red950: return "#450a0a";

    case TailwindColor::Orange50:  return "#fff7ed";
    case TailwindColor::Orange100: return "#ffedd5";
    case TailwindColor::Orange200: return "#fed7aa";
    case TailwindColor::Orange300: return "#fdba74";
    case TailwindColor::Orange400: return "#fb923c";
    case TailwindColor::Orange500: return "#f97316";
    case TailwindColor::Orange600: return "#ea580c";
    case TailwindColor::Orange700: return "#c2410c";
    case TailwindColor::Orange800: return "#9a3412";
    case TailwindColor::Orange900: return "#7c2d12";
    case TailwindColor::Orange950: return "#431407";

    case TailwindColor::Amber50:  return "#fffbeb";
    case TailwindColor::Amber100: return "#fef3c7";
    case TailwindColor::Amber200: return "#fde68a";
    case TailwindColor::Amber300: return "#fcd34d";
    case TailwindColor::Amber400: return "#fbbf24";
    case TailwindColor::Amber500: return "#f59e0b";
    case TailwindColor::Amber600: return "#d97706";
    case TailwindColor::Amber700: return "#b45309";
    case TailwindColor::Amber800: return "#92400e";
    case TailwindColor::Amber900: return "#78350f";
    case TailwindColor::Amber950: return "#451a03";

    case TailwindColor::Yellow50:  return "#fefce8";
    case TailwindColor::Yellow100: return "#fef9c3";
    case TailwindColor::Yellow200: return "#fef08a";
    case TailwindColor::Yellow300: return "#fde047";
    case TailwindColor::Yellow400: return "#facc15";
    case TailwindColor::Yellow500: return "#eab308";
    case TailwindColor::Yellow600: return "#ca8a04";
    case TailwindColor::Yellow700: return "#a16207";
    case TailwindColor::Yellow800: return "#854d0e";
    case TailwindColor::Yellow900: return "#713f12";
    case TailwindColor::Yellow950: return "#422006";

    case TailwindColor::Lime50:  return "#f7fee7";
    case TailwindColor::Lime100: return "#ecfccb";
    case TailwindColor::Lime200: return "#d9f99d";
    case TailwindColor::Lime300: return "#bef264";
    case TailwindColor::Lime400: return "#a3e635";
    case TailwindColor::Lime500: return "#84cc16";
    case TailwindColor::Lime600: return "#65a30d";
    case TailwindColor::Lime700: return "#4d7c0f";
    case TailwindColor::Lime800: return "#3f6212";
    case TailwindColor::Lime900: return "#365314";
    case TailwindColor::Lime950: return "#1a2e05";

    case TailwindColor::Green50:  return "#f0fdf4";
    case TailwindColor::Green100: return "#dcfce7";
    case TailwindColor::Green200: return "#bbf7d0";
    case TailwindColor::Green300: return "#86efac";
    case TailwindColor::Green400: return "#4ade80";
    case TailwindColor::Green500: return "#22c55e";
    case TailwindColor::Green600: return "#16a34a";
    case TailwindColor::Green700: return "#15803d";
    case TailwindColor::Green800: return "#166534";
    case TailwindColor::Green900: return "#14532d";
    case TailwindColor::Green950: return "#052e16";

    case TailwindColor::Emerald50:  return "#ecfdf5";
    case TailwindColor::Emerald100: return "#d1fae5";
    case TailwindColor::Emerald200: return "#a7f3d0";
    case TailwindColor::Emerald300: return "#6ee7b7";
    case TailwindColor::Emerald400: return "#34d399";
    case TailwindColor::Emerald500: return "#10b981";
    case TailwindColor::Emerald600: return "#059669";
    case TailwindColor::Emerald700: return "#047857";
    case TailwindColor::Emerald800: return "#065f46";
    case TailwindColor::Emerald900: return "#064e3b";
    case TailwindColor::Emerald950: return "#022c22";

    case TailwindColor::Teal50:  return "#f0fdfa";
    case TailwindColor::Teal100: return "#ccfbf1";
    case TailwindColor::Teal200: return "#99f6e4";
    case TailwindColor::Teal300: return "#5eead4";
    case TailwindColor::Teal400: return "#2dd4bf";
    case TailwindColor::Teal500: return "#14b8a6";
    case TailwindColor::Teal600: return "#0d9488";
    case TailwindColor::Teal700: return "#0f766e";
    case TailwindColor::Teal800: return "#115e59";
    case TailwindColor::Teal900: return "#134e4a";
    case TailwindColor::Teal950: return "#042f2e";

    case TailwindColor::Cyan50:  return "#ecfeff";
    case TailwindColor::Cyan100: return "#cffafe";
    case TailwindColor::Cyan200: return "#a5f3fc";
    case TailwindColor::Cyan300: return "#67e8f9";
    case TailwindColor::Cyan400: return "#22d3ee";
    case TailwindColor::Cyan500: return "#06b6d4";
    case TailwindColor::Cyan600: return "#0891b2";
    case TailwindColor::Cyan700: return "#0e7490";
    case TailwindColor::Cyan800: return "#155e75";
    case TailwindColor::Cyan900: return "#164e63";
    case TailwindColor::Cyan950: return "#083344";

    case TailwindColor::Sky50:  return "#f0f9ff";
    case TailwindColor::Sky100: return "#e0f2fe";
    case TailwindColor::Sky200: return "#bae6fd";
    case TailwindColor::Sky300: return "#7dd3fc";
    case TailwindColor::Sky400: return "#38bdf8";
    case TailwindColor::Sky500: return "#0ea5e9";
    case TailwindColor::Sky600: return "#0284c7";
    case TailwindColor::Sky700: return "#0369a1";
    case TailwindColor::Sky800: return "#075985";
    case TailwindColor::Sky900: return "#0c4a6e";
    case TailwindColor::Sky950: return "#082f49";

    case TailwindColor::Blue50:  return "#eff6ff";
    case TailwindColor::Blue100: return "#dbeafe";
    case TailwindColor::Blue200: return "#bfdbfe";
    case TailwindColor::Blue300: return "#93c5fd";
    case TailwindColor::Blue400: return "#60a5fa";
    case TailwindColor::Blue500: return "#3b82f6";
    case TailwindColor::Blue600: return "#2563eb";
    case TailwindColor::Blue700: return "#1d4ed8";
    case TailwindColor::Blue800: return "#1e40af";
    case TailwindColor::Blue900: return "#1e3a8a";
    case TailwindColor::Blue950: return "#172554";

    case TailwindColor::Indigo50:  return "#eef2ff";
    case TailwindColor::Indigo100: return "#e0e7ff";
    case TailwindColor::Indigo200: return "#c7d2fe";
    case TailwindColor::Indigo300: return "#a5b4fc";
    case TailwindColor::Indigo400: return "#818cf8";
    case TailwindColor::Indigo500: return "#6366f1";
    case TailwindColor::Indigo600: return "#4f46e5";
    case TailwindColor::Indigo700: return "#4338ca";
    case TailwindColor::Indigo800: return "#3730a3";
    case TailwindColor::Indigo900: return "#312e81";
    case TailwindColor::Indigo950: return "#1e1b4b";

    case TailwindColor::Violet50:  return "#f5f3ff";
    case TailwindColor::Violet100: return "#ede9fe";
    case TailwindColor::Violet200: return "#ddd6fe";
    case TailwindColor::Violet300: return "#c4b5fd";
    case TailwindColor::Violet400: return "#a78bfa";
    case TailwindColor::Violet500: return "#8b5cf6";
    case TailwindColor::Violet600: return "#7c3aed";
    case TailwindColor::Violet700: return "#6d28d9";
    case TailwindColor::Violet800: return "#5b21b6";
    case TailwindColor::Violet900: return "#4c1d95";
    case TailwindColor::Violet950: return "#2e1065";

    case TailwindColor::Purple50:  return "#faf5ff";
    case TailwindColor::Purple100: return "#f3e8ff";
    case TailwindColor::Purple200: return "#e9d5ff";
    case TailwindColor::Purple300: return "#d8b4fe";
    case TailwindColor::Purple400: return "#c084fc";
    case TailwindColor::Purple500: return "#a855f7";
    case TailwindColor::Purple600: return "#9333ea";
    case TailwindColor::Purple700: return "#7e22ce";
    case TailwindColor::Purple800: return "#6b21a8";
    case TailwindColor::Purple900: return "#581c87";
    case TailwindColor::Purple950: return "#3b0764";

    case TailwindColor::Fuchsia50:  return "#fdf4ff";
    case TailwindColor::Fuchsia100: return "#fae8ff";
    case TailwindColor::Fuchsia200: return "#f5d0fe";
    case TailwindColor::Fuchsia300: return "#f0abfc";
    case TailwindColor::Fuchsia400: return "#e879f9";
    case TailwindColor::Fuchsia500: return "#d946ef";
    case TailwindColor::Fuchsia600: return "#c026d3";
    case TailwindColor::Fuchsia700: return "#a21caf";
    case TailwindColor::Fuchsia800: return "#86198f";
    case TailwindColor::Fuchsia900: return "#701a75";
    case TailwindColor::Fuchsia950: return "#4a044e";

    case TailwindColor::Pink50:  return "#fdf2f8";
    case TailwindColor::Pink100: return "#fce7f3";
    case TailwindColor::Pink200: return "#fbcfe8";
    case TailwindColor::Pink300: return "#f9a8d4";
    case TailwindColor::Pink400: return "#f472b6";
    case TailwindColor::Pink500: return "#ec4899";
    case TailwindColor::Pink600: return "#db2777";
    case TailwindColor::Pink700: return "#be185d";
    case TailwindColor::Pink800: return "#9d174d";
    case TailwindColor::Pink900: return "#831843";
    case TailwindColor::Pink950: return "#500724";

    case TailwindColor::Rose50:  return "#fff1f2";
    case TailwindColor::Rose100: return "#ffe4e6";
    case TailwindColor::Rose200: return "#fecdd3";
    case TailwindColor::Rose300: return "#fda4af";
    case TailwindColor::Rose400: return "#fb7185";
    case TailwindColor::Rose500: return "#f43f5e";
    case TailwindColor::Rose600: return "#e11d48";
    case TailwindColor::Rose700: return "#be123c";
    case TailwindColor::Rose800: return "#9f1239";
    case TailwindColor::Rose900: return "#881337";
    case TailwindColor::Rose950: return "#4c0519";

    default:
        throw std::runtime_error("Invalid Tailwind color");
    }
    // clang-format on
}

template<typename V, typename C = float>
constexpr V tailwindColorToRgb(const TailwindColor c)
{
    return KDUtils::hexToRgb<V, C>(tailwindColorToHex(c));
}

template<typename V, typename C = float>
constexpr V tailwindColorToRgba(const TailwindColor c, const C alpha)
{
    return KDUtils::hexToRgba<V, C>(tailwindColorToHex(c), alpha);
}

} // namespace KDUtils
