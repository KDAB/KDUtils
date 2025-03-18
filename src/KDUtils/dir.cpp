/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dir.h"
#include "logging.h"

#include <whereami.h>

#include <filesystem>
#include <iostream>

namespace KDUtils {

Dir::Dir()
{
}

Dir::Dir(const char *path)
    : m_path(std::filesystem::u8path(path))
{
    // If the filename part is empty, the parameter was likely supplied
    // with a trailing separator. If so, strip it by going to the parent.
    if (m_path.filename().empty()) {
        m_path = m_path.parent_path();
    }
}

Dir::Dir(const std::string &path)
    : Dir(path.c_str())
{
}

Dir::Dir(const std::filesystem::path &path)
    : Dir(path.string())
{
}

bool Dir::exists() const
{
    std::error_code e;
    return std::filesystem::exists(m_path, e) && std::filesystem::is_directory(m_path, e);
}

bool Dir::mkdir(const MkDirOptions &options)
{
    if (options.createParentDirectories) {
        auto parentToMake = parent();
        if (!parentToMake.exists() && !parentToMake.mkdir(options)) {
            SPDLOG_CRITICAL("Failed to create parent directory {}", parentToMake.path());
            return false;
        }
    }

    std::error_code e;
    return std::filesystem::create_directory(m_path, e);
}

bool Dir::rmdir()
{
    std::error_code e;
    return std::filesystem::remove_all(m_path, e) > 0;
}

std::string Dir::path() const
{
    return m_path.generic_u8string();
}

std::string Dir::dirName() const
{
    return m_path.filename().generic_u8string();
}

std::string Dir::absoluteFilePath(const std::string &file) const
{
    std::error_code e;
    const std::filesystem::path fPath{ file };
    return std::filesystem::absolute(m_path / fPath, e).generic_u8string();
}

Dir Dir::applicationDir()
{
    const int length = wai_getExecutablePath(NULL, 0, NULL); // NOLINT(modernize-use-nullptr)
    if (length >= 0) {
        std::string appPath;
        appPath.resize(length);
        wai_getExecutablePath(appPath.data(), length, NULL); // NOLINT(modernize-use-nullptr)

        const std::filesystem::path appFSPath(appPath);
        return Dir(appFSPath.parent_path());
    }

    return {};
}

std::string Dir::fromNativeSeparators(const std::string &path)
{
    return std::filesystem::path(path).generic_u8string();
}

bool Dir::operator==(const Dir &other) const
{
    return m_path == other.m_path;
}

Dir Dir::parent() const
{
    auto absolutePath = std::filesystem::absolute(m_path);
    if (!absolutePath.has_parent_path())
        SPDLOG_CRITICAL("Parent path not found for {}", m_path.generic_u8string());

    return Dir(absolutePath.parent_path());
}

bool Dir::hasParent() const
{
    auto absolutePath = std::filesystem::absolute(m_path);
    auto parentPath = absolutePath.parent_path();
    return absolutePath != parentPath;
}

} // namespace KDUtils
