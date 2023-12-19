/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>

namespace KDFoundation {

class Event;

class KDFOUNDATION_API EventReceiver
{
public:
    virtual ~EventReceiver() noexcept;

    virtual void event(EventReceiver *target, Event *ev){};
};

} // namespace KDFoundation
