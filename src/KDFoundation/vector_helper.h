/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <vector>
#include <thread>
#include <algorithm>
#include <KDFoundation/config.h>

namespace KDFoundation {

template<typename T, typename U>
inline void moveAtEnd(std::vector<T> &destination, std::vector<U> &&source)
{
    destination.insert(destination.end(),
                       std::make_move_iterator(source.begin()),
                       std::make_move_iterator(source.end()));
}

template<typename T>
inline T moveAndClear(T &data)
{
    T ret(std::move(data));
    data.clear();
    return ret;
}

template<typename T, typename U>
inline void append(std::vector<T> &destination, const U &source)
{
    destination.insert(destination.end(),
                       source.cbegin(),
                       source.cend());
}

template<typename T, typename U>
inline bool contains(const std::vector<T> &destination, const U &element) noexcept
{
    return std::find(destination.begin(), destination.end(), element) != destination.end();
}

template<typename ForwardIterator>
inline void deleteAll(ForwardIterator begin, ForwardIterator end)
{
    while (begin != end) {
        delete *begin;
        ++begin;
    }
}

template<typename Container>
inline void deleteAll(const Container &c)
{
    deleteAll(c.begin(), c.end());
}

template<typename T>
inline size_t indexOf(const std::vector<T> &destination, const T &element)
{
    auto it = std::find(destination.cbegin(), destination.cend(), element);
    if (it == destination.cend()) {
        return -1;
    }
    return std::distance(destination.cbegin(), it);
}

} // namespace KDFoundation
