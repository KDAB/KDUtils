/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#ifndef KDUTILS_FILE_H
#define KDUTILS_FILE_H

#include <KDUtils/kdutils_export.h>
#include <KDUtils/bytearray.h>
#include <iostream>
#include <fstream>
#include <filesystem>

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
    std::fstream m_stream;
};

} // namespace KDUtils

#endif // KUESA_COREUTILS_FILE_H
