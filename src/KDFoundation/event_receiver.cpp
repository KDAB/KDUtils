/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Marco Thaller <marco.thaller@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "event_receiver.h"
#include "core_application.h"

using namespace KDFoundation;

EventReceiver::~EventReceiver() noexcept
{
    if (CoreApplication::instance() != nullptr)
        CoreApplication::instance()->removeAllEventsTargeting(*this);
}
