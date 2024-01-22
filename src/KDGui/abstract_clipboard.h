/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Joshua Goins <joshua.goins@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>

#include <KDGui/abstract_platform_window.h>

namespace KDGui {

class AbstractGuiPlatformIntegration;

class AbstractClipboard : public KDFoundation::Object
{
public:
    virtual std::string text() = 0;
    virtual void setText(std::string_view text) = 0;
};

} // namespace KDGui
