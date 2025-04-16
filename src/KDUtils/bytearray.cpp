/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "bytearray.h"
#include <cstring>
#include <algorithm>
#include <array>

namespace KDUtils {

ByteArray::ByteArray()
{
}

// Note: be careful, the data we store is never null terminated.
// We provide the size if needed so that strn* functions can be used.
ByteArray::ByteArray(const char *s, size_t size)
{
    if (s == nullptr)
        return;
    if (size == 0)
        size = std::strlen(s);
    m_data.resize(size);
    std::memcpy(m_data.data(), s, size);
}

ByteArray::ByteArray(const uint8_t *s, size_t size)
{
    if (s == nullptr)
        return;
    m_data.resize(size);
    std::memcpy(m_data.data(), s, size);
}

ByteArray::ByteArray(const std::vector<uint8_t> &data)
    : m_data(data)
{
}

ByteArray::ByteArray(size_t size, uint8_t c)
{
    m_data.resize(size);
    std::memset(m_data.data(), c, size);
}

ByteArray::ByteArray(const std::string &data)
{
    m_data.resize(data.size());
    std::memcpy(m_data.data(), data.data(), data.size());
}

ByteArray::ByteArray(const ByteArray &other)
    : m_data(other.m_data)
{
}

ByteArray::~ByteArray()
{
}

ByteArray &ByteArray::operator=(const ByteArray &other)
{
    if (this != &other)
        m_data = other.m_data;
    return *this;
}

ByteArray &ByteArray::operator=(const char *s)
{
    const size_t len = std::strlen(s);
    m_data = {};
    m_data.resize(len);
    std::memcpy(m_data.data(), s, len);
    return *this;
}

ByteArray ByteArray::mid(size_t pos, size_t len) const
{
    if (pos >= size())
        return {};
    if (len == 0)
        len = size() - pos;
    len = std::min(len, size());
    return ByteArray({ m_data.begin() + int64_t(pos), m_data.begin() + int64_t(pos + len) });
}

ByteArray ByteArray::left(size_t left) const
{
    left = std::min(left, size());
    return ByteArray({ m_data.begin(), m_data.begin() + int64_t(left) });
}

int64_t ByteArray::indexOf(uint8_t v) const
{
    auto it = std::find(m_data.begin(), m_data.end(), v);
    if (it == m_data.end())
        return -1;
    return std::distance(m_data.begin(), it);
}

ByteArray &ByteArray::remove(size_t pos, size_t len)
{
    if (pos >= size())
        return *this;
    len = std::min(size() - pos, len);
    m_data.erase(m_data.begin() + int64_t(pos), m_data.begin() + int64_t(pos + len));
    return *this;
}

ByteArray &ByteArray::operator+=(const ByteArray &other)
{
    m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
    return *this;
}

void ByteArray::append(const ByteArray &other)
{
    m_data.insert(m_data.end(), other.m_data.begin(), other.m_data.end());
}

void ByteArray::append(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0)
        return;
    m_data.insert(m_data.end(), data, data + size);
}

void ByteArray::append(uint8_t c)
{
    m_data.push_back(c);
}

void ByteArray::append(const std::string &data)
{
    if (data.empty())
        return;
    m_data.insert(m_data.end(), data.data(), data.data() + data.size());
}

void ByteArray::clear()
{
    m_data.clear();
}

size_t ByteArray::size() const
{
    return m_data.size();
}

void ByteArray::reserve(size_t size)
{
    m_data.reserve(size);
}

void ByteArray::resize(size_t size)
{
    m_data.resize(size);
}

bool ByteArray::empty() const
{
    return size() == size_t(0);
}

bool ByteArray::startsWith(const ByteArray &b) const
{
    if (b.size() > size())
        return false;
    return std::memcmp(constData(), b.constData(), b.size()) == 0;
}

bool ByteArray::endsWith(const ByteArray &b) const
{
    if (b.size() > size())
        return false;
    return std::memcmp(constData() + size() - b.size(), b.constData(), b.size()) == 0;
}

ByteArray::ByteArray(ByteArray &&other) noexcept
    : m_data(std::move(other.m_data))
{
}

ByteArray::ByteArray(std::vector<uint8_t> &&data)
    : m_data(std::move(data))
{
}

ByteArray &ByteArray::operator=(ByteArray &&other) noexcept
{
    if (this != &other)
        m_data = std::move(other.m_data);
    return *this;
}

// Base64 handling inspired from:
// https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/
// https://www.boost.org/doc/libs/1_66_0/boost/beast/core/detail/base64.hpp

ByteArray ByteArray::toBase64() const
{
    const char *base64 = "ABCDEFGH"
                         "IJKLMNOP"
                         "QRSTUVWX"
                         "YZabcdef"
                         "ghijklmn"
                         "opqrstuv"
                         "wxyz0123"
                         "456789+/";
    const uint8_t padchar = '=';

    ByteArray encoded((size() + 2) / 3 * 4, 0);
    std::vector<uint8_t> &out = encoded.m_data;

    int64_t outIdx = 0;
    int64_t inIdx = 0;
    const size_t len = size();

    for (size_t c = 0, m = len / 3; c < m; ++c, inIdx += 3) {
        // Handle 3 input bytes at once
        out[outIdx++] = base64[(m_data[inIdx] & 0xfc) >> 2];
        out[outIdx++] = base64[((m_data[inIdx] & 0x03) << 4) + ((m_data[inIdx + 1] & 0xf0) >> 4)];
        out[outIdx++] = base64[((m_data[inIdx + 2] & 0xc0) >> 6) + ((m_data[inIdx + 1] & 0x0f) << 2)];
        out[outIdx++] = base64[m_data[inIdx + 2] & 0x3f];
    }

    // Handle padding at the end
    const size_t padding = len % 3;
    switch (padding) {
    case 2:
        out[outIdx++] = base64[(m_data[inIdx] & 0xfc) >> 2];
        out[outIdx++] = base64[((m_data[inIdx] & 0x03) << 4) + ((m_data[inIdx + 1] & 0xf0) >> 4)];
        out[outIdx++] = base64[(m_data[inIdx + 1] & 0x0f) << 2];
        out[outIdx++] = padchar;
        break;

    case 1:
        out[outIdx++] = base64[(m_data[inIdx] & 0xfc) >> 2];
        out[outIdx++] = base64[((m_data[inIdx] & 0x03) << 4)];
        out[outIdx++] = padchar;
        out[outIdx++] = padchar;
        break;

    case 0:
    default: // silence clang-tidy
        break;
    }

    return encoded;
}

namespace {

constexpr int8_t getInverseFromB64(uint8_t c)
{
    if (c >= 'A' && c <= 'Z')
        return static_cast<int8_t>(c - 'A');
    if (c >= 'a' && c <= 'z')
        return static_cast<int8_t>(c - 'a' + 26);
    if (c >= '0' && c <= '9')
        return static_cast<int8_t>(c - '0' + 52);
    if (c == '+' || c == '-')
        return 62;
    if (c == '/' || c == '_')
        return 63;
    return -1;
}

int8_t *getInverseB64Table()
{
    static std::array<int8_t, 255> table;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;
        for (uint8_t i = 0; i < 255; ++i)
            table[i] = getInverseFromB64(i);
    }
    return table.data();
}

} // namespace

ByteArray ByteArray::fromBase64(const ByteArray &base64)
{
    const int8_t *inverseB64Table = getInverseB64Table();
    const uint8_t padchar = '=';
    ByteArray decoded((base64.size() * 3) / 4);
    std::vector<uint8_t> &out = decoded.m_data;

    int64_t outIdx = 0;
    int64_t inIdx = 0;
    std::array<uint8_t, 4> pending4Bytes;

    for (size_t i = 0, m = base64.size(); i < m; ++i) {
        // Have we reached right most padding
        const uint8_t c = base64.m_data[i];
        if (c == padchar)
            break;
        // Find inverse of base64 character
        const int8_t inverse = inverseB64Table[c];
        // If we get -1, we have a character which isn' t part of the base64 alphabet -> break out
        if (inverse < 0)
            break;

        pending4Bytes[inIdx++] = uint8_t(inverse); // inverse is necesarrely positive here
        if (inIdx == 4) {
            // Decode 4 base64 bytes at once -> results in 3 bytes
            out[outIdx++] = (pending4Bytes[0] << 2) + ((pending4Bytes[1] & 0x30) >> 4);
            out[outIdx++] = ((pending4Bytes[1] & 0xf) << 4) + ((pending4Bytes[2] & 0x3c) >> 2);
            out[outIdx++] = ((pending4Bytes[2] & 0x3) << 6) + pending4Bytes[3];
            inIdx = 0;
        }
    }

    // We might not have fully decoded the last pending4Bytes
    if (inIdx > 1) {
        out[outIdx++] = (pending4Bytes[0] << 2) + ((pending4Bytes[1] & 0x30) >> 4);
        if (inIdx > 2)
            out[outIdx++] = ((pending4Bytes[1] & 0xf) << 4) + ((pending4Bytes[2] & 0x3c) >> 2);
        if (inIdx > 3)
            out[outIdx++] = ((pending4Bytes[2] & 0x3) << 6) + pending4Bytes[3];
    }

    // Truncate
    decoded.resize(outIdx);

    return decoded;
}

bool operator==(const ByteArray &a, const ByteArray &b)
{
    return a.vector() == b.vector();
}

bool operator!=(const ByteArray &a, const ByteArray &b)
{
    return !(a == b);
}

ByteArray operator+(const ByteArray &a, const ByteArray &b)
{
    return ByteArray(a) += b;
}

} // namespace KDUtils
