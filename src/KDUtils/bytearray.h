/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDUTILS_BYTEARRAY_H
#define KDUTILS_BYTEARRAY_H

#include <vector>
#include <cstdint>
#include <string>

#include <KDUtils/kdutils_global.h>

namespace KDUtils {

class KDUTILS_API ByteArray
{
public:
    ByteArray();
    explicit ByteArray(const char *, size_t size = 0);
    explicit ByteArray(const uint8_t *, size_t size);
    explicit ByteArray(const std::vector<uint8_t> &data);
    explicit ByteArray(size_t size, uint8_t c = 0);
    explicit ByteArray(const std::string &data);
    ByteArray(const ByteArray &);
    ~ByteArray();

    ByteArray &operator=(const ByteArray &);
    ByteArray &operator=(const char *);

    ByteArray(ByteArray &&other) noexcept;
    explicit ByteArray(std::vector<uint8_t> &&data);
    ByteArray &operator=(ByteArray &&other) noexcept;

    ByteArray mid(size_t pos, size_t len = 0) const;
    ByteArray left(size_t left) const;
    ByteArray &remove(size_t pos, size_t len);
    ByteArray &operator+=(const ByteArray &other);
    void append(const ByteArray &other);
    void append(const uint8_t *data, size_t size);
    void clear();

    size_t size() const;
    void reserve(size_t size);
    void resize(size_t size);
    bool empty() const;
    inline bool isEmpty() const { return empty(); }

    int64_t indexOf(uint8_t) const;

    bool startsWith(const ByteArray &b) const;
    bool endsWith(const ByteArray &b) const;

    inline uint8_t *data() { return m_data.data(); }
    inline const uint8_t *data() const { return m_data.data(); }
    inline const uint8_t *constData() const { return m_data.data(); }
    inline const std::vector<uint8_t> &vector() const { return m_data; }

    std::string toStdString() const { return std::string(m_data.begin(), m_data.end()); }

    ByteArray toBase64() const;
    static ByteArray fromBase64(const ByteArray &base64);

private:
    std::vector<uint8_t> m_data;
};

KDUTILS_API bool operator==(const ByteArray &a, const ByteArray &b);
KDUTILS_API bool operator!=(const ByteArray &a, const ByteArray &b);
KDUTILS_API ByteArray operator+(const ByteArray &a, const ByteArray &b);

} // namespace KDUtils

#endif // KUESA_COREUTILS_BYTEARRAY_H
