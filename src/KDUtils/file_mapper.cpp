/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/file_mapper.h>
#include <spdlog/spdlog.h>
#include <variant>
#include <mio/mmap.hpp>

namespace KDUtils {
using ReadMap = mio::basic_mmap<mio::access_mode::read, uint8_t>;
using WriteMap = mio::basic_mmap<mio::access_mode::write, uint8_t>;

struct FileMapper::Map {
    std::variant<ReadMap, WriteMap> map;

    ~Map() {};

    Map(bool writeable)
    {
        // NOTE: mio's maps consist of primitive types which won't throw. but
        // the constructors are not marked noexcept
        if (writeable)
            map = WriteMap();
        else
            map = ReadMap();
    }

    [[nodiscard]] bool writable() const noexcept
    {
        return std::holds_alternative<WriteMap>(map);
    }

    WriteMap &writableMap()
    {
        return std::get<WriteMap>(map);
    }

    ReadMap &readOnlyMap()
    {
        return std::get<ReadMap>(map);
    }
};

FileMapper::FileMapper(File &&file)
    : m_path(file.path()), m_map(nullptr)
{
    if (file.isOpen())
        file.close();
}

// this has to be here instead of in the header because the compiler cannot make
// a default deleter without knowing the size of the type.
FileMapper::~FileMapper() = default;

namespace {
template<std::ios::openmode mode>
const uint8_t *mapImpl(
        std::unique_ptr<FileMapper::Map> &output,
        const std::string &path,
        std::uintmax_t offset,
        std::uintmax_t length)
{
    std::error_code error;
    constexpr bool writableRequested = static_cast<bool>(mode & std::ios::out);

    auto warnMapped = []() {
        SPDLOG_WARN("Requested map data from a FileMapper which was already mapped.");
    };

    // handle possible invalid output states
    if (output == nullptr || output->writable() != writableRequested) {
        // map is nonexistent or of the wrong type
        output = std::make_unique<FileMapper::Map>(writableRequested);
    } else {
        // in this case, we already had a map in the correct mode
        // go ahead and bail if we're already mapped
        if constexpr (writableRequested) {
            if (output->writableMap().is_mapped()) {
                warnMapped();
                return output->writableMap().data();
            }
        } else {
            if (output->readOnlyMap().is_mapped()) {
                warnMapped();
                return output->readOnlyMap().data();
            }
        }
    }

    if constexpr (writableRequested) {
        output->writableMap().map(path, offset, length, error);
        if (error) {
            return nullptr;
        }
        return output->writableMap().data();
    } else {
        output->readOnlyMap().map(path, offset, length, error);
        if (error) {
            return nullptr;
        }
        return output->readOnlyMap().data();
    }
}
} // namespace

const uint8_t *FileMapper::map(std::uintmax_t offset, std::uintmax_t length) const
{
    try {
        // fake const here to make the API use this overload when you have a const FileMapper
        return mapImpl<std::ios::in>(const_cast<std::unique_ptr<Map> &>(m_map), m_path, offset, length);
    } catch (std::system_error &e) {
        // thrown by the non-default constructors, which we currently don't call.
        SPDLOG_ERROR("mio file mapping err. Error code {}: {}", e.code().value(), e.code().message());
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

uint8_t *FileMapper::map(std::uintmax_t offset, std::uintmax_t length)
{
    try {
        return const_cast<uint8_t *>(mapImpl<std::ios::out>(m_map, m_path, offset, length));
    } catch (std::system_error &e) {
        SPDLOG_ERROR("mio file mapping err. Error code {}: {}", e.code().value(), e.code().message());
        return nullptr;
    } catch (...) {
        SPDLOG_ERROR("Unknown memory mapping error");
        return nullptr;
    }
}

bool FileMapper::unmap(const uint8_t *mapping)
{
    if (m_map == nullptr) {
        SPDLOG_WARN("Requested an unmap of a FileMapper which was never mapped.");
        return false;
    }

    if (mapping != nullptr) {
        // it's inexpensive to request the data of an existing mapping, so lets do
        // it just to make sure its the same as the one being passed in
        const uint8_t *existingMapping;
        if (m_map->writable()) {
            try {
                existingMapping = map();
            } catch (...) {
                SPDLOG_ERROR("Unknown memory mapping error");
                return false;
            }
        } else {
            try {
                uint8_t *writable = map();
                existingMapping = writable;
            } catch (...) {
                SPDLOG_ERROR("Unknown memory mapping error");
                return false;
            }
        }

        if (existingMapping != mapping) {
            SPDLOG_WARN("Pointer passed to FileMapper::unmap which does not match existing.");
        }
    }

    try {
        if (m_map->writable()) {
            std::error_code error;
            m_map->writableMap().sync(error);
            if (error) {
                return false;
            }
            m_map->writableMap().unmap();
        } else {
            m_map->readOnlyMap().unmap();
        }
    } catch (std::system_error &e) {
        SPDLOG_ERROR("mio file mapping err. Error code {}: {}", e.code().value(), e.code().message());
        return false;
    } catch (...) {
        SPDLOG_ERROR("Unknown memory mapping error");
        return false;
    }

    return true;
}

const std::string &FileMapper::path() const
{
    return m_path;
}
std::uintmax_t FileMapper::size() const
{
    if (m_map == nullptr) {
        SPDLOG_WARN("Queried the size of an unmapped FileMapper. This usually "
                    "indicates calling size() and map() in the same line with "
                    "unexpected execution order.");
        return 0;
    } else if (m_map->writable()) {
        return m_map->writableMap().size();
    } else {
        return m_map->readOnlyMap().size();
    }
}

} // namespace KDUtils
