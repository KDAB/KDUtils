/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <cstdint>

#ifdef ANDROID
#  include <spdlog/sinks/android_sink.h>
#endif

namespace KDFoundation {

enum class LogLevel : std::uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

inline std::shared_ptr<spdlog::logger> createLogger(const std::string &name)
{
#ifdef ANDROID
    return spdlog::android_logger_mt(name, "serenity");
#else
    return spdlog::stdout_color_mt(name);
#endif
}

} // namespace KDFoundation
