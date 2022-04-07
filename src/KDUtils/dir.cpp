/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dir.h"
#include <filesystem>

#include <whereami.h>

#include <iostream>

namespace KDUtils {

Dir::Dir()
{
}

Dir::Dir(const char *path)
    : m_path(path)
{
}

Dir::Dir(const std::string &path)
    : m_path(path)
{
}

Dir::Dir(const std::filesystem::path &path)
    : m_path(path.string())
{
}

bool Dir::exists() const
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::exists(dPath, e) && std::filesystem::is_directory(m_path, e);
}

bool Dir::mkdir()
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::create_directory(dPath, e);
}

bool Dir::rmdir()
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    return std::filesystem::remove_all(dPath, e) > 0;
}

const std::string &Dir::path() const
{
    return m_path;
}

std::string Dir::dirName() const
{
    const std::filesystem::path p{ m_path };
    return p.parent_path().filename().u8string();
}

std::string Dir::absoluteFilePath(const std::string &file) const
{
    std::error_code e;
    const std::filesystem::path dPath{ m_path };
    const std::filesystem::path fPath{ file };
    return std::filesystem::absolute(dPath / fPath, e).u8string();
}

Dir Dir::applicationDir()
{
    const int length = wai_getExecutablePath(NULL, 0, NULL); // NOLINT(modernize-use-nullptr)
    if (length >= 0) {
        std::string appPath;
        appPath.resize(length);
        wai_getExecutablePath(appPath.data(), length, NULL); // NOLINT(modernize-use-nullptr)

        const std::filesystem::path appFSPath(appPath);
        return Dir(appFSPath.parent_path().append(""));
    }

    return {};
}

bool Dir::operator==(const Dir &other) const
{
    return std::filesystem::path(m_path) == std::filesystem::path(other.m_path);
}

} // namespace KDUtils
