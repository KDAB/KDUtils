/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "url.h"
#include "dir.h"
#include <regex>

using namespace std::string_literals;

namespace KDUtils {

Url::Url(const std::string &url)
    : m_url(url)
{
    const std::regex rexExp(u8"(?:([^\\/]{2,})?:(?:\\/\\/)?)?(.*\\/)*(.+\\..+)?"s);
    std::smatch match;
    const bool hasMatch = std::regex_match(m_url, match, rexExp);
    if (hasMatch) {
        m_scheme = match[1].str();
        m_path = match[2].str();
        m_fileName = match[3].str();
    }
}

bool Url::isLocalFile() const
{
    return m_scheme.rfind(u8"file"s, 0) == 0;
}

std::string Url::toLocalFile() const
{
    if (!isLocalFile())
        return {};
    return m_path + m_fileName;
}

const std::string &Url::scheme() const
{
    return m_scheme;
}

const std::string &Url::fileName() const
{
    return m_fileName;
}

const std::string &Url::path() const
{
    return m_path;
}

const std::string &Url::url() const
{
    return m_url;
}

Url Url::fromLocalFile(const std::string &url)
{
    std::string path = Dir::fromNativeSeparators(url);
    Url u(path);
    // If url has a scheme, we return it
    if (!u.scheme().empty())
        return u;
    // Do we hold a path?
    if (u.path().empty())
        return Url(std::string("file:") + path);
    const bool isWindowsPath = path.size() > 1 && path[1] == ':' && path[0] != '/';
    if (isWindowsPath)
        path.insert(path.begin(), '/');
    return Url(std::string("file://") + path);
}

bool operator==(const Url &a, const Url &b)
{
    return a.url() == b.url();
}

bool operator!=(const Url &a, const Url &b)
{
    return !(a == b);
}

} // namespace KDUtils
