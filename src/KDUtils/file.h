/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#pragma once

#include <KDUtils/kdutils_global.h>
#include <KDUtils/bytearray.h>
#include <iostream>
#include <filesystem>

namespace KDUtils {

// Some platforms have special requirements for accessing some file types
// such as assets and shared data directories on Android.
// For Desktop platforms, type should always be Normal.
enum class StorageType {
    Normal, // Files that can be accessed normally using the c++ standard library
    Asset, // Files that must be accessed from an application's embedded assets
};

struct PlatformFileData;

class KDUTILS_API File
{
public:
    File(const std::string &path, StorageType type = StorageType::Normal);
    ~File();

    // Can't be copied
    File(File &) = delete;
    File &operator=(File &) = delete;

    bool exists() const;
    static bool exists(const std::string &path, StorageType type = StorageType::Normal);

    bool open(std::ios_base::openmode mode);
    bool isOpen() const;
    void flush();
    void close();
    bool remove();

    ByteArray readAll();
    void write(const ByteArray &data);
    std::string fileName() const;
    const std::string &path() const;
    StorageType type() const;

    std::uintmax_t size() const;
    static std::uintmax_t size(const std::string &path, StorageType type = StorageType::Normal);

private:
    std::string m_path;
    std::unique_ptr<PlatformFileData> m_data;
    StorageType m_type = StorageType::Normal;
};

} // namespace KDUtils
