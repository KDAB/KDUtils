/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/file_mapper.h>
#include <spdlog/spdlog.h>
#include <variant>
// ensure mio does not throw
#undef __cpp_exceptions
#include <mio/mmap.hpp>

namespace KDUtils {
using ReadMap = mio::basic_mmap<mio::access_mode::read, uint8_t>;
using WriteMap = mio::basic_mmap<mio::access_mode::write, uint8_t>;

struct FileMapper::Map {
    std::variant<ReadMap, WriteMap> map;

    inline ~Map(){};

    inline Map(bool writeable)
    {
        if (writeable)
            map = WriteMap();
        else
            map = ReadMap();
    }

    [[nodiscard]] inline bool writable() const
    {
        return std::holds_alternative<WriteMap>(map);
    }

    inline WriteMap &writableMap()
    {
        return std::get<WriteMap>(map);
    }

    inline ReadMap &readOnlyMap()
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

template<std::ios::openmode mode>
static const uint8_t *mapImpl(
        std::unique_ptr<FileMapper::Map> &output,
        const std::string &path,
        std::uintmax_t offset,
        std::uintmax_t length)
{
    std::error_code error;
    constexpr bool writableRequested = mode & std::ios::out;

    auto warnMapped = []() {
        SPDLOG_WARN("Requested map data from a FileMapper which was already mapped.");
    };

    // handle possible invalid output states
    if (output == nullptr) {
        output = std::make_unique<FileMapper::Map>(writableRequested);
    } else if (output->writable() != writableRequested) {
        // we already have a map but its of the wrong type
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

const uint8_t *FileMapper::map(std::uintmax_t offset, std::uintmax_t length) const
{
    // fake const here to make the API use this overload when you have a const FileMapper
    return mapImpl<std::ios::in>(const_cast<std::unique_ptr<Map> &>(m_map), m_path, offset, length);
}

uint8_t *FileMapper::map(std::uintmax_t offset, std::uintmax_t length)
{
    return const_cast<uint8_t *>(mapImpl<std::ios::out>(m_map, m_path, offset, length));
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
            existingMapping = map();
        } else {
            uint8_t *writable = map();
            existingMapping = writable;
        }

        if (existingMapping != mapping) {
            SPDLOG_WARN("Pointer passed to FileMapper::unmap which does not match existing.");
        }
    }

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
