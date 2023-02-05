/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_export.h>
#include <KDUtils/kdutils_global.h>

#define KDFOUNDATION_API KDFOUNDATION_EXPORT

#include <string>

namespace KDFoundation {

inline std::string assetPath()
{
#if defined(ASSET_PATH)
    return ASSET_PATH;
#else
    return "";
#endif
}

inline std::string largeAssetPath()
{
#if defined(LARGE_ASSET_PATH)
    return LARGE_ASSET_PATH;
#else
    return "";
#endif
}

} // namespace KDFoundation
