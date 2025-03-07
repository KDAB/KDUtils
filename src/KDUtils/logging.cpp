/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "logging.h"
#if defined(ANDROID)
#include <spdlog/sinks/android_sink.h>
#elif defined(_WIN32)
#include <spdlog/sinks/msvc_sink.h>
#endif

namespace KDUtils {

Logger::LoggerFactoryFunction Logger::ms_loggerFactory = {};

std::shared_ptr<spdlog::logger> Logger::logger(const std::string &name, spdlog::level::level_enum defaultLevel)
{
    std::shared_ptr<spdlog::logger> logger;
    if (ms_loggerFactory) {
        // Use the factory set by the application which should check
        // its own spdlog registry first before creating a new logger.
        logger = ms_loggerFactory(name, defaultLevel);
    } else {
        // No factory set, use the spdlog registry from KDUtils
        logger = spdlog::get(name);
        if (!logger) {
#if defined(ANDROID)
            logger = spdlog::android_logger_mt(name, name);
#elif defined(_WIN32)
            // Create both msvc_sink and stdout_color_sink
            auto msvc_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list{ msvc_sink, console_sink });
#else
            logger = spdlog::stdout_color_mt(name);
#endif
            logger->set_level(defaultLevel);
        }
    }
    return logger;
}

void Logger::setLoggerFactory(const LoggerFactoryFunction &factory)
{
    ms_loggerFactory = factory;
}

Logger::LoggerFactoryFunction Logger::loggerFactory()
{
    return ms_loggerFactory;
}

} // namespace KDUtils
