/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <chrono>
#include <thread>
#include <tuple>

template<typename... Args>
class SignalSpy
{
public:
    template<typename Signal>
    explicit SignalSpy(Signal &s)
    {
        m_connection = s.connect([this](Args... args) {
            callback(args...);
        });
    }

    bool isValid() const
    {
        return true;
    }

    std::tuple<Args...> &args()
    {
        return m_args;
    }

    void clear()
    {
        m_args = {};
        m_count = 0;
    }

    uint32_t count() const
    {
        return m_count;
    }

    using milliseconds = std::chrono::duration<int64_t, std::milli>;

    bool wait(milliseconds timeout = milliseconds(5000))
    {
        milliseconds elapsed;
        const auto step = milliseconds({ timeout / 5 });
        const uint32_t refCount = m_count;
        while (elapsed < timeout) {
            std::this_thread::sleep_for(step);
            if (m_count > refCount)
                return true;
            elapsed += step;
        }
        return false;
    }

private:
    void callback(Args... args)
    {
        m_args = std::make_tuple(args...);
        ++m_count;
    }

    uint32_t m_count = 0;
    std::tuple<Args...> m_args;
    KDBindings::ScopedConnection m_connection;
};
