/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <chrono>

#include <kdbindings/property.h>

#include <KDFoundation/kdfoundation_global.h>

namespace KDFoundation {

class AbstractPlatformTimer;

class KDFOUNDATION_API Timer
{
public:
    explicit Timer();
    ~Timer();

    KDBindings::Signal<> timeout;

    KDBindings::Property<bool> running{ false };
    KDBindings::Property<std::chrono::microseconds> interval{};

private:
    std::unique_ptr<AbstractPlatformTimer> m_platformTimer;
};

} // namespace KDFoundation
