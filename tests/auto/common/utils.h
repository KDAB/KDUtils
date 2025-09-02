/*
 * This file is part of KDUtils.
 *
 * SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
 * Author: Anton Kreuzkamp <anton.kreuzkamp@kdab.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Contact KDAB at <info@kdab.com> for commercial licensing options.
 */

#pragma once

#include <KDFoundation/event.h>
#include <KDFoundation/event_loop.h>
#include <KDFoundation/event_receiver.h>
#include <KDFoundation/object.h>

#include <doctest.h>

auto shouldFailOnMacOS()
{
#if defined(PLATFORM_MACOS)
    return doctest::should_fail(true);
#else
    return doctest::should_fail(false);
#endif
}
