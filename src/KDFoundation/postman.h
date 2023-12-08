/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "KDFoundation/event_receiver.h"
#include <KDFoundation/kdfoundation_global.h>

#include <vector>

namespace KDFoundation {

class Event;
class Object;

class KDFOUNDATION_API Postman
{
public:
    void deliverEvent(EventReceiver *target, Event *event);

    void addFilter(Object *filter);
    void removeFilter(Object *filter);
    const std::vector<Object *> &filters() const { return m_filters; }

private:
    std::vector<Object *> m_filters;
};

} // namespace KDFoundation
