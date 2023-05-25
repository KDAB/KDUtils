/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDUTILS_LOGGER_H
#define KDUTILS_LOGGER_H

#include <spdlog/spdlog.h>
#if defined(ANDROID)
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

namespace KDUtils {

class Logger
{
public:
    enum class Type {
        Debug,
        Warn
    };

    explicit Logger(const char *category, Logger::Type type)
        : m_category(category)
        , m_logger(spdlog::get(category))
        , m_type(type)
    {
        // Create logger if it wasn't already (use multithread logger just to be safe)
        if (!m_logger)
#if defined(ANDROID)
            m_logger = spdlog::android_logger_mt(category, category);
#else
            m_logger = spdlog::stdout_color_mt(category);
#endif

        switch (m_type) {
        case Logger::Type::Debug:
            m_logLevel = spdlog::level::debug;
            break;
        case Logger::Type::Warn:
            m_logLevel = spdlog::level::warn;
            break;
        }
    }

    Logger::Type type() const
    {
        return m_type;
    }

    template<typename... Args>
    using FormatString = fmt::format_string<Args...>;

    // For actual logging
    template<typename... Args>
    void warn(FormatString<Args...> fmt, Args &&...args)
    {
        m_logger->warn(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(FormatString<Args...> fmt, Args &&...args)
    {
        m_logger->debug(fmt, std::forward<Args>(args)...);
    }

    // For cWarning/cDebug macros
    Logger warn() const
    {
        return Logger(m_category, m_type);
    }
    Logger debug() const
    {
        return Logger(m_category, m_type);
    }

    template<typename... Args>
    Logger warn(FormatString<Args...> fmt, Args &&...args) const
    {
        Logger l(m_category, m_type);
        l.warn(fmt, std::forward<Args>(args)...);
        return l;
    }

    template<typename... Args>
    Logger debug(FormatString<Args...> fmt, Args &&...args) const
    {
        Logger l(m_category, m_type);
        l.debug(fmt, std::forward<Args>(args)...);
        return l;
    }

    template<typename T>
    Logger &operator<<(const T &v)
    {
        m_logger->log(m_logLevel, "{}", v);
        return *this;
    }

    Logger &operator<<(const char *s)
    {
        m_logger->log(m_logLevel, "{}", s);
        return *this;
    }

private:
    const char *m_category;
    std::shared_ptr<spdlog::logger> m_logger;
    Logger::Type m_type;
    spdlog::level::level_enum m_logLevel = spdlog::level::debug;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_LOGGER_H
