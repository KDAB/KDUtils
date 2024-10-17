/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#ifndef KDUTILS_FILE_MAPPER_H
#define KDUTILS_FILE_MAPPER_H
#include <KDUtils/file.h>
#include <KDUtils/kdutils_global.h>

namespace KDUtils {
/// Provides memory mapping from a file.
class KDUTILS_API FileMapper
{
public:
    struct Map;

    FileMapper(File &&);
    ~FileMapper();
    // can be moved
    FileMapper(FileMapper &&) = default;
    FileMapper &operator=(FileMapper &&) = default;
    // cannot be copied
    FileMapper(FileMapper &) = delete;
    FileMapper &operator=(FileMapper &) = delete;

    /// Produces a pointer to read-only mapped data. Becomes invalid when the FileMapper is destroyed.
    /// Closes and makes invalid any existing mappings.
    /// @arg offset: the start of the mapping relative to the beginning of the file, in bytes.
    /// @arg length: the number of bytes after the start of the mapping to map
    /// @return: the pointer to the mapped data or nullptr on failure.
    const uint8_t *map(std::uintmax_t offset = 0, std::uintmax_t length = 0) const;

    /// Produces a pointer to writable mapped data. Becomes invalid when the FileMapper is destroyed.
    /// Closes and makes invalid any existing mappings.
    /// @arg offset: the start of the mapping relative to the beginning of the file, in bytes.
    /// @arg length: the number of bytes after the start of the mapping to map
    /// @return: the pointer to the mapped data or nullptr on failure.
    uint8_t *map(std::uintmax_t offset = 0, std::uintmax_t length = 0);

    /// Closes the existing mapping.
    /// @arg mapping: The memory acquired with `map`.
    /// May also be nullptr.
    /// @return: true on success, false on IO failure or invalid pointer.
    bool unmap(const uint8_t *mapping);

    const std::string &path() const;

    /// Returns the size of the *mapping*. May differ from the size of the file.
    std::uintmax_t size() const;

private:
    std::string m_path;
    std::unique_ptr<Map> m_map;
};
} // namespace KDUtils

#endif
