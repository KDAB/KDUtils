/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "file.h"

namespace KDUtils {

File::File(const std::string &path)
    : m_path(path)
{
}

File::~File()
{
    if (isOpen())
        close();
}

bool File::exists() const
{
    std::error_code e;
    return std::filesystem::exists(m_path, e) && std::filesystem::is_regular_file(m_path, e);
}

bool File::open(std::ios_base::openmode mode)
{
    if (isOpen())
        close();
    m_stream.open(m_path, mode);
    return m_stream.is_open();
}

bool File::isOpen() const
{
    return m_stream.is_open();
}

void File::flush()
{
    if (isOpen())
        m_stream.flush();
}

void File::close()
{
    flush();
    m_stream.close();
}

bool File::remove()
{
    if (exists()) {
        std::error_code e;
        return std::filesystem::remove(m_path, e);
    }
    return false;
}

ByteArray File::readAll()
{
    if (!isOpen())
        return {};
    const size_t s = size();
    if (s == 0)
        return {};
    ByteArray b;
    b.resize(s);
    // Move to beginning and read all
    m_stream.seekg(0);
    m_stream.read(reinterpret_cast<char *>(b.data()), s);
    return b;
}

void File::write(const ByteArray &data)
{
    if (isOpen())
        m_stream.write(reinterpret_cast<const char *>(data.constData()), data.size());
}

std::string File::fileName() const
{
    std::filesystem::path p(m_path);
    return p.filename().u8string();
}

const std::string &File::path() const
{
    return m_path;
}

std::uintmax_t File::size() const
{
    std::error_code e;
    return std::filesystem::file_size(m_path, e);
}

} // namespace KDUtils
