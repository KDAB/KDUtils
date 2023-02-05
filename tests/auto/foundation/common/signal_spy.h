/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <chrono>
#include <thread>

class SignalSpy
{
public:
    template<typename Signal>
    explicit SignalSpy(Signal &s)
    {
        s.connect(&SignalSpy::callback, this);
    }

    bool isValid() const
    {
        return true;
    }

    void clear()
    {
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
    void callback()
    {
        ++m_count;
    }

    uint32_t m_count = 0;
};
