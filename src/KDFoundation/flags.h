/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <iostream>
#include <numeric>
#include <string>
#include <type_traits>

namespace KDFoundation {

template<
        typename EnumType,
        typename Enable = std::enable_if_t<std::is_enum_v<EnumType>, typename std::underlying_type<EnumType>::type>>
class Flags
{
public:
    using enum_type = EnumType;
    using storage_type = typename std::underlying_type<EnumType>::type;

    Flags() noexcept
        : m_flags{ 0 }
    {
    }

    explicit Flags(storage_type value) noexcept
        : m_flags{ value }
    {
    }

    Flags(enum_type value) noexcept
        : m_flags{ static_cast<storage_type>(value) }
    {
    }

    Flags(Flags const &other) = default;
    Flags &operator=(Flags const &other) = default;

    Flags(Flags &&other) = default;
    Flags &operator=(Flags &&other) = default;

    Flags &operator=(enum_type value) noexcept
    {
        *this = Flags{ value };
        return *this;
    }

    // Explicit conversion operator
    operator storage_type() const noexcept
    {
        return m_flags;
    }

    operator std::string() const
    {
        return toString();
    }

    bool operator[](enum_type flag) const noexcept
    {
        return test(flag);
    }

    std::string toString() const
    {
        const auto s = size();
        std::string str(s, '0');
        for (std::size_t x = 0; x < s; ++x)
            str[s - x - 1] = (m_flags & (static_cast<storage_type>(1) << x) ? '1' : '0');
        return str;
    }

    Flags &setAll() noexcept
    {
        m_flags = ~storage_type(0);
        return *this;
    }

    Flags &set(enum_type flag, bool val = true) noexcept
    {
        const auto f = static_cast<storage_type>(flag);
        m_flags = (val ? (m_flags | f) : (m_flags & ~f));
        return *this;
    }

    Flags &clear() noexcept
    {
        m_flags = storage_type{ 0 };
        return *this;
    }

    Flags &unset(enum_type flag) noexcept
    {
        m_flags &= ~static_cast<storage_type>(flag);
        return *this;
    }

    Flags &invert() noexcept
    {
        m_flags = ~m_flags;
        return *this;
    }

    Flags inverse() const noexcept
    {
        Flags f{ *this };
        return f.invert();
    }

    Flags &toggle(enum_type flag) noexcept
    {
        m_flags ^= static_cast<storage_type>(flag);
        return *this;
    }

    std::size_t count() const noexcept
    {
        // http://www-graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
        storage_type bits = m_flags;
        std::size_t total = 0;
        for (; bits != 0; ++total)
            bits &= bits - 1; // clear the least significant bit set
        return total;
    }

    constexpr std::size_t size() const noexcept { return sizeof(enum_type) * 8; }
    bool test(enum_type flag) const noexcept { return (m_flags & static_cast<storage_type>(flag)) > 0; }
    bool any() const noexcept { return m_flags > 0; }
    bool none() const noexcept { return m_flags == 0; }

    bool operator==(enum_type value) const noexcept
    {
        return m_flags == static_cast<storage_type>(value);
    }

    Flags &operator|=(enum_type flag)
    {
        m_flags = m_flags | flag;
        return *this;
    }

    Flags &operator&=(enum_type flag)
    {
        m_flags = m_flags & flag;
        return *this;
    }

private:
    storage_type m_flags;
};

// This one maintains the type of enums when bitwise OR'ing strongly typed enums.
template<typename T>
auto operator|(T a, T b) -> std::enable_if_t<std::is_enum_v<T>, T>
{
    using storage_type = typename std::underlying_type<T>::type;
    return static_cast<T>(static_cast<storage_type>(a) | static_cast<storage_type>(b));
}

template<typename T>
auto operator|(Flags<T> a, T b) -> std::enable_if_t<std::is_enum_v<T>, Flags<T>>
{
    using storage_type = typename std::underlying_type<T>::type;
    return Flags<T>(a.operator storage_type() | static_cast<storage_type>(b));
}

template<typename T>
auto operator|(T a, Flags<T> b) -> std::enable_if_t<std::is_enum_v<T>, Flags<T>>
{
    using storage_type = typename std::underlying_type<T>::type;
    return Flags<T>(static_cast<storage_type>(a) | b.operator storage_type());
}

template<typename EnumType>
Flags<EnumType> operator&(const Flags<EnumType> &lhs, const Flags<EnumType> &rhs)
{
    return Flags<EnumType>(typename Flags<EnumType>::storage_type(lhs) & typename Flags<EnumType>::storage_type(rhs));
}

template<typename EnumType>
Flags<EnumType> operator|(const Flags<EnumType> &lhs, const Flags<EnumType> &rhs)
{
    return Flags<EnumType>(typename Flags<EnumType>::storage_type(lhs) | typename Flags<EnumType>::storage_type(rhs));
}

template<typename EnumType>
Flags<EnumType> operator^(const Flags<EnumType> &lhs, const Flags<EnumType> &rhs)
{
    return Flags<EnumType>(typename Flags<EnumType>::storage_type(lhs) ^ typename Flags<EnumType>::storage_type(rhs));
}

template<class charT, class traits, typename EnumType>
std::basic_ostream<charT, traits> &operator<<(std::basic_ostream<charT, traits> &os, const Flags<EnumType> &flags)
{
    return os << flags.toString();
}

} // namespace KDFoundation
