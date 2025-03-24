/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#pragma once

#include <KDUtils/kdutils_global.h>

#include <android/asset_manager.h>

namespace KDUtils {

KDUTILS_API void setAssetManager(AAssetManager *assetManager);
KDUTILS_API AAssetManager *assetManager();

} // namespace KDUtils
