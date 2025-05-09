/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "file.h"

#include <KDUtils/logging.h>

#include <fstream>

namespace KDUtils {

struct PlatformFileData {
    std::fstream stream;
};

File::File(const std::string &path, StorageType type)
    : m_path(path)
    , m_type(type)
    , m_data(new PlatformFileData())
{
}

File::~File()
{
    try {
        if (isOpen())
            close();
    } catch (...) {
        SPDLOG_WARN("Exception caught when destroying file handle, path: {}", m_path);
    }
}

bool File::exists() const
{
    return exists(m_path, m_type);
}

bool File::exists(const std::string &path, StorageType)
{
    // StorageType is not required on desktop platforms
    std::error_code e;
    return std::filesystem::exists(path, e) && std::filesystem::is_regular_file(path, e);
}

bool File::open(std::ios_base::openmode mode)
{
    if (isOpen())
        close();

    m_data->stream.open(m_path, mode);
    return m_data->stream.is_open();
}

bool File::isOpen() const
{
    return m_data->stream.is_open();
}

void File::flush()
{
    if (isOpen())
        m_data->stream.flush();
}

void File::close()
{
    flush();
    m_data->stream.close();
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
    m_data->stream.seekg(0);
    m_data->stream.read(reinterpret_cast<char *>(b.data()), static_cast<std::streamsize>(s));
    return b;
}

void File::write(const ByteArray &data)
{
    if (isOpen())
        m_data->stream.write(reinterpret_cast<const char *>(data.constData()), static_cast<std::streamsize>(data.size()));
}

std::string File::fileName() const
{
    const std::filesystem::path p(m_path);
    return p.filename().u8string();
}

const std::string &File::path() const
{
    return m_path;
}

std::uintmax_t File::size() const
{
    return size(m_path, m_type);
}

std::uintmax_t File::size(const std::string &path, StorageType)
{
    // StorageType is not required on desktop platforms
    std::error_code e;
    return std::filesystem::file_size(path, e);
}

StorageType File::type() const
{
    return m_type;
}

} // namespace KDUtils
