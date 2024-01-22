/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "timer.h"

#include "core_application.h"
#include "platform/abstract_platform_integration.h"
#include "platform/abstract_platform_timer.h"

using namespace KDFoundation;

Timer::Timer()
    : m_platformTimer(CoreApplication::instance()->eventLoop()->createPlatformTimer(this))
{
}

Timer::~Timer()
{
}
