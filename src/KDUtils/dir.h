/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDUTILS_DIR_H
#define KDUTILS_DIR_H

#include <KDUtils/kdutils_global.h>
#include <KDUtils/file.h>
#include <string>
#include <filesystem>

namespace KDUtils {

class KDUTILS_API Dir
{
public:
    Dir();
    Dir(const char *path, StorageType type = StorageType::Normal);
    Dir(const std::string &path, StorageType type = StorageType::Normal);
    Dir(const std::filesystem::path &path, StorageType type = StorageType::Normal);

    bool exists() const;

    struct MkDirOptions {
        bool createParentDirectories{ false };
    };
    bool mkdir(const MkDirOptions &options = { false });
    bool rmdir();

    std::string path() const;
    std::string dirName() const;
    std::string absoluteFilePath(const std::string &file) const;
    StorageType type() const;

    File file(const std::string &fileName) const;

    Dir parent() const;
    bool hasParent() const;

    // Returns a directory based on a relative path from this directory, preserving the storage type
    Dir relativeDir(const std::string &relativePath) const;

    static Dir applicationDir();
    static std::string fromNativeSeparators(const std::string &path);

    // Returns a normalized form of the directory with lowercase drive letters on Windows
    Dir normalized() const;

    bool operator==(const Dir &other) const;

private:
    std::filesystem::path m_path;
    StorageType m_type = StorageType::Normal;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_DIR_H
