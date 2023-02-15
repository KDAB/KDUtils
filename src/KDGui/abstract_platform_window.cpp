/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/abstract_platform_window.h>
#include "window.h"

#include <cassert>

using namespace KDGui;

AbstractPlatformWindow::AbstractPlatformWindow(Window *window)
    : m_window{ window }
{
    assert(window);
    window->title.valueChanged().connect(&AbstractPlatformWindow::setTitle, this);
}
