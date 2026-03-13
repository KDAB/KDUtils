/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "android_file.h"

#include <KDUtils/file.h>
#include <KDUtils/logging.h>

#include <fstream>

namespace KDUtils {

namespace {
AAssetManager *s_assetManager = nullptr;

const char *assetPath(const std::string &path)
{
    const char *p = path.c_str();
    if (*p == '/')
        ++p;
    return p;
}
} // namespace

struct PlatformFileData {
    AAsset *asset = nullptr;
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

bool File::exists(const std::string &path, StorageType type)
{
    if (type == StorageType::Normal) {
        std::error_code e;
        return std::filesystem::exists(path, e) && std::filesystem::is_regular_file(path, e);
    } else {
        if (!s_assetManager)
            return false;
        const std::filesystem::path p(path);
        const std::string parentPath = p.parent_path().u8string();
        const std::string fileName = p.filename().u8string();
        bool found = false;
        if (AAssetDir *dir = AAssetManager_openDir(s_assetManager, assetPath(parentPath))) {
            while (const char *next = AAssetDir_getNextFileName(dir)) {
                if (fileName == next) {
                    found = true;
                    break;
                }
            }
            AAssetDir_close(dir);
        }
        return found;
    }
}

bool File::open(std::ios_base::openmode mode)
{
    if (isOpen())
        close();
    if (m_type == StorageType::Normal) {
        // Android cannot seem to cope with reopening a stream in a different mode
        m_data->stream = {};
        m_data->stream.open(m_path, mode);
        return m_data->stream.is_open();
    } else {
        if (!s_assetManager)
            return false;
        m_data->asset = AAssetManager_open(s_assetManager, assetPath(m_path), AASSET_MODE_UNKNOWN);
        return m_data->asset != nullptr;
    }
}

bool File::isOpen() const
{
    if (m_type == StorageType::Normal) {
        return m_data->stream.is_open();
    } else {
        return m_data->asset != nullptr;
    }
}

void File::flush()
{
    if (m_type == StorageType::Normal) {
        if (isOpen())
            m_data->stream.flush();
    }
}

void File::close()
{
    if (m_type == StorageType::Normal) {
        flush();
        m_data->stream.close();
    } else {
        if (m_data->asset) {
            AAsset_close(m_data->asset);
            m_data->asset = nullptr;
        }
    }
}

bool File::remove()
{
    if (m_type == StorageType::Normal) {
        if (exists()) {
            std::error_code e;
            return std::filesystem::remove(m_path, e);
        }
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
    if (m_type == StorageType::Normal) {
        m_data->stream.seekg(0);
        m_data->stream.read(reinterpret_cast<char *>(b.data()), static_cast<std::streamsize>(s));
    } else {
        AAsset_seek(m_data->asset, 0, SEEK_SET);
        AAsset_read(m_data->asset, reinterpret_cast<char *>(b.data()), s);
    }
    return b;
}

void File::write(const ByteArray &data)
{
    if (m_type == StorageType::Normal) {
        if (isOpen())
            m_data->stream.write(reinterpret_cast<const char *>(data.constData()), static_cast<std::streamsize>(data.size()));
    }
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

std::uintmax_t File::size(const std::string &path, StorageType type)
{
    if (type == StorageType::Normal) {
        std::error_code e;
        return std::filesystem::file_size(path, e);
    } else {
        if (!s_assetManager)
            return 0;
        AAsset *asset = AAssetManager_open(s_assetManager, assetPath(path), AASSET_MODE_UNKNOWN);
        if (!asset)
            return 0;
        const auto s = AAsset_getLength(asset);
        AAsset_close(asset);
        return s;
    }
}

StorageType File::type() const
{
    return m_type;
}

void setAssetManager(AAssetManager *assetManager)
{
    s_assetManager = assetManager;
}

AAssetManager *assetManager()
{
    return s_assetManager;
}

} // namespace KDUtils
