/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <vector>
#include <thread>
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

using BucketIndices = std::tuple<size_t, size_t, size_t>; // startIdx, endIdx, bucketIdx

template<typename Container>
std::vector<BucketIndices> splitIntoBuckets(const Container &container, uint32_t idealBucketSize)
{
    const size_t containerSize = container.size();
    uint32_t maxAvailableThreads = 1;
    if constexpr (USE_THREADS)
        maxAvailableThreads = std::thread::hardware_concurrency();
    const uint32_t actualBucketSize = std::max(size_t(idealBucketSize), containerSize / maxAvailableThreads);
    const size_t bucketCount = std::max(size_t(1), containerSize / actualBucketSize);

    std::vector<BucketIndices> buckets;
    buckets.reserve(bucketCount);

    for (auto i = 0; i < bucketCount - 1; ++i) {
        const size_t startIdx = i * actualBucketSize;
        buckets.emplace_back(startIdx, startIdx + actualBucketSize, i);
    }
    const size_t lastStartIdx = (bucketCount - 1) * actualBucketSize;
    buckets.emplace_back(lastStartIdx, lastStartIdx + (containerSize - lastStartIdx), bucketCount - 1);
    return buckets;
}

} // namespace KDFoundation
