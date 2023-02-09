/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDUTILS_ELAPSEDTIMER_H
#define KDUTILS_ELAPSEDTIMER_H

#include <KDUtils/kdutils_export.h>
#include <chrono>
#include <ratio>

namespace KDUtils {

class KDUTILS_EXPORT ElapsedTimer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using NsecDuration = std::chrono::duration<int64_t, std::nano>;
    using MsecDuration = std::chrono::duration<int64_t, std::milli>;

    ElapsedTimer();

    NsecDuration elapsed() const;
    int64_t msecElapsed() const;
    int64_t nsecElapsed() const; // Compatibility with Qt API

    NsecDuration restart();
    void start();

private:
    TimePoint m_startTimePoint = Clock::now();
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_ELAPSEDTIMER_H
