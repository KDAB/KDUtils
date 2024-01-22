/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDGui/abstract_platform_window.h>
#include "window.h"

#include <cassert>

using namespace KDGui;

AbstractPlatformWindow::AbstractPlatformWindow(Window *window, AbstractPlatformWindow::Type type)
    : m_window{ window }
    , m_type(type)
{
    assert(window);
    window->title.valueChanged().connect(&AbstractPlatformWindow::setTitle, this);
}

AbstractPlatformWindow::Type AbstractPlatformWindow::type() const
{
    return m_type;
}
