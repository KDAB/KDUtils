/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logging.h"

namespace KDUtils {

Logger::LoggerFactoryFunction Logger::ms_loggerFactory = {};

std::shared_ptr<spdlog::logger> Logger::logger(const std::string &name)
{
    std::shared_ptr<spdlog::logger> logger;
    if (ms_loggerFactory) {
        logger = ms_loggerFactory(name);
    } else {
#if defined(ANDROID)
        logger = spdlog::android_logger_mt(name, name);
#else
        logger = spdlog::stdout_color_mt(name);
#endif
    }
    return logger;
}

void Logger::setLoggerFactory(LoggerFactoryFunction factory)
{
    ms_loggerFactory = factory;
}

Logger::LoggerFactoryFunction Logger::loggerFactory()
{
    return ms_loggerFactory;
}

} // namespace KDUtils
