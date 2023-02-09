/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#ifndef KDUTILS_FILE_H
#define KDUTILS_FILE_H

#include <KDUtils/kdutils_export.h>
#include <KDUtils/bytearray.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#if defined(ANDROID)
#include <android/asset_manager.h>
#endif

namespace KDUtils {

class KDUTILS_EXPORT File
{
public:
    File(const std::string &path);
    ~File();

    // Can't be copied
    File(File &) = delete;
    File &operator=(File &) = delete;

    bool exists() const;

    bool open(std::ios_base::openmode mode);
    bool isOpen() const;
    void flush();
    void close();
    bool remove();

    ByteArray readAll();
    void write(const ByteArray &data);
    std::string fileName() const;
    const std::string &path() const;

    std::uintmax_t size() const;

private:
    std::string m_path;
#if defined(ANDROID)
    AAsset *m_asset = nullptr;
#else
    std::fstream m_stream;
#endif
};

#if defined(ANDROID)
KDUTILS_EXPORT void setAssetManager(AAssetManager *assetManager);
#endif

} // namespace KDUtils

#endif // KUESA_COREUTILS_FILE_H
