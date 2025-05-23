/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDUtils/kdutils_global.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <functional>

namespace KDUtils {

class KDUTILS_API Logger
{
public:
    static std::shared_ptr<spdlog::logger> logger(const std::string &name, spdlog::level::level_enum defaultLevel = spdlog::level::warn);

    using LoggerFactoryFunction = std::function<std::shared_ptr<spdlog::logger>(const std::string &, spdlog::level::level_enum)>;

    static void setLoggerFactory(const LoggerFactoryFunction &factory);
    static LoggerFactoryFunction loggerFactory();

private:
    static LoggerFactoryFunction ms_loggerFactory;
};

} // namespace KDUtils
