/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "file.h"

namespace KDUtils {

#if defined(ANDROID)
static AAssetManager *s_assetManager = nullptr;

const char *assetPath(const std::string &path)
{
    const char *p = path.c_str();
    if (*p == '/')
        ++p;
    return p;
}
#endif

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
#if !defined(ANDROID)
    std::error_code e;
    return std::filesystem::exists(m_path, e) && std::filesystem::is_regular_file(m_path, e);
#else
    if (!s_assetManager)
        return false;
    const std::filesystem::path p(m_path);
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
#endif
}

bool File::open(std::ios_base::openmode mode)
{
    if (isOpen())
        close();
#if !defined(ANDROID)
    m_stream.open(m_path, mode);
    return m_stream.is_open();
#else
    if (!s_assetManager)
        return false;
    m_asset = AAssetManager_open(s_assetManager, assetPath(m_path), AASSET_MODE_UNKNOWN);
    return m_asset != nullptr;
#endif
}

bool File::isOpen() const
{
#if !defined(ANDROID)
    return m_stream.is_open();
#else
    return m_asset != nullptr;
#endif
}

void File::flush()
{
#if !defined(ANDROID)
    if (isOpen())
        m_stream.flush();
#endif
}

void File::close()
{
#if !defined(ANDROID)
    flush();
    m_stream.close();
#else
    if (m_asset) {
        AAsset_close(m_asset);
        m_asset = nullptr;
    }
#endif
}

bool File::remove()
{
#if !defined(ANDROID)
    if (exists()) {
        std::error_code e;
        return std::filesystem::remove(m_path, e);
    }
#endif
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
#if !defined(ANDROID)
    m_stream.seekg(0);
    m_stream.read(reinterpret_cast<char *>(b.data()), static_cast<std::streamsize>(s));
#else
    AAsset_seek(m_asset, 0, SEEK_SET);
    AAsset_read(m_asset, reinterpret_cast<char *>(b.data()), s);
#endif
    return b;
}

void File::write(const ByteArray &data)
{
#if !defined(ANDROID)
    if (isOpen())
        m_stream.write(reinterpret_cast<const char *>(data.constData()), static_cast<std::streamsize>(data.size()));
#endif
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
#if !defined(ANDROID)
    std::error_code e;
    return std::filesystem::file_size(m_path, e);
#else
    if (!s_assetManager)
        return 0;
    AAsset *asset = AAssetManager_open(s_assetManager, assetPath(m_path), AASSET_MODE_UNKNOWN);
    if (!asset)
        return 0;
    const auto s = AAsset_getLength(asset);
    AAsset_close(asset);
    return s;
#endif
}

#if defined(ANDROID)
void setAssetManager(AAssetManager *assetManager)
{
    s_assetManager = assetManager;
}
#endif

} // namespace KDUtils
