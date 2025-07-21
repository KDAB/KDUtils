/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "dir.h"
#include "logging.h"

#ifdef ANDROID
#include <KDUtils/platform/android/android_file.h>
#endif

#include <whereami.h>

#include <filesystem>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace KDUtils {

Dir::Dir()
{
}

Dir::Dir(const char *path, StorageType type)
    : m_path(std::filesystem::u8path(path))
    , m_type(type)
{
    // If the filename part is empty, the parameter was likely supplied
    // with a trailing separator. If so, strip it by going to the parent.
    if (m_path.filename().empty()) {
        m_path = m_path.parent_path();
    }
}

Dir::Dir(const std::string &path, StorageType type)
    : Dir(path.c_str(), type)
{
}

Dir::Dir(const std::filesystem::path &path, StorageType type)
    : Dir(path.string(), type)
{
}

bool Dir::exists() const
{
#ifdef ANDROID
    if (m_type == StorageType::Asset) {
        if (AAssetDir *dir = AAssetManager_openDir(assetManager(), m_path.c_str())) {
            AAssetDir_close(dir);
            return true;
        } else {
            return false;
        }
    } else
#endif
    {
        std::error_code e;
        return std::filesystem::exists(m_path, e) && std::filesystem::is_directory(m_path, e);
    }
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

StorageType Dir::type() const
{
    return m_type;
}

Dir Dir::applicationDir()
{
    const int length = wai_getExecutablePath(NULL, 0, NULL); // NOLINT(modernize-use-nullptr)
    if (length >= 0) {
        std::string appPath;
        appPath.resize(length);
        wai_getExecutablePath(appPath.data(), length, NULL); // NOLINT(modernize-use-nullptr)

        const std::filesystem::path appFSPath(appPath);
        return Dir(appFSPath.parent_path().generic_u8string());
    }

    return {};
}

std::string Dir::fromNativeSeparators(const std::string &path)
{
    return std::filesystem::path(path).generic_u8string();
}

Dir Dir::normalized() const
{
    Dir result(*this);
    std::string pathStr = result.m_path.string();

    // Convert Windows drive letter to lowercase if present
    if (pathStr.length() >= 2 && pathStr[1] == ':' && std::isalpha(static_cast<unsigned char>(pathStr[0]))) {
        pathStr[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(pathStr[0])));
    }

    // Convert the path to use the generic format with forward slashes
    result.m_path = std::filesystem::path(pathStr).make_preferred().generic_u8string();

    return result;
}

bool Dir::operator==(const Dir &other) const
{
    // Compare normalized paths to handle case differences in Windows drive letters
    return this->normalized().m_path == other.normalized().m_path;
}

Dir Dir::parent() const
{
    auto absolutePath = std::filesystem::absolute(m_path);
    if (!absolutePath.has_parent_path()) {
        SPDLOG_CRITICAL("Parent path not found for {}", m_path.generic_u8string());
        return {};
    }

    return Dir(absolutePath.parent_path());
}

bool Dir::hasParent() const
{
    auto absolutePath = std::filesystem::absolute(m_path);
    auto parentPath = absolutePath.parent_path();
    return absolutePath != parentPath;
}

File Dir::file(const std::string &fileName) const
{
    return File((m_path / fileName).generic_u8string(), m_type);
}

Dir Dir::relativeDir(const std::string &relativePath) const
{
    return Dir(m_path / relativePath, m_type);
}

} // namespace KDUtils
