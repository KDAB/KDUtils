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
#include <string>
#include <filesystem>

namespace KDUtils {

class KDUTILS_API Dir
{
public:
    Dir();
    Dir(const char *path);
    Dir(const std::string &path);
    Dir(const std::filesystem::path &path);

    bool exists() const;
    bool mkdir();
    bool rmdir();
    std::string path() const;
    std::string dirName() const;
    std::string absoluteFilePath(const std::string &file) const;

    Dir parent() const;
    bool hasParent() const;

    static Dir applicationDir();
    static std::string fromNativeSeparators(const std::string &path);

    bool operator==(const Dir &other) const;

private:
    std::filesystem::path m_path;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_DIR_H
