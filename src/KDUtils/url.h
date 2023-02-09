/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDUTILS_URL_H
#define KDUTILS_URL_H

#include <KDUtils/kdutils_export.h>
#include <string>

namespace KDUtils {

class KDUTILS_EXPORT Url
{
public:
    Url() = default;
    explicit Url(const std::string &url);

    bool isLocalFile() const;
    std::string toLocalFile() const;
    std::string scheme() const;
    std::string fileName() const;
    std::string path() const;
    std::string url() const;
    bool empty() const { return m_url.empty(); }
    bool isEmpty() const { return empty(); }

    static Url fromLocalFile(const std::string &url);

private:
    std::string m_url;
    std::string m_fileName;
    std::string m_scheme;
    std::string m_path;
};

KDUTILS_EXPORT bool operator==(const Url &a, const Url &b);
KDUTILS_EXPORT bool operator!=(const Url &a, const Url &b);

} // namespace KDUtils

#endif // KUESA_COREUTILS_URL_H
