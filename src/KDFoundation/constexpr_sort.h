/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <array>
#include <iterator>

// C++17 constexpr sort as described at
// https://tristanbrindle.com/posts/a-more-useful-compile-time-quicksort
//
// Can be used as:
//
// constexpr auto a = KDFoundation::sort(std::array { 2, 5, 1, 8, 4 });
//

namespace KDFoundation {

template<class ForwardIt1, class ForwardIt2>
constexpr void iter_swap(ForwardIt1 a, ForwardIt2 b)
{
    auto temp = std::move(*a);
    *a = std::move(*b);
    *b = std::move(temp);
}

template<class InputIt, class UnaryPredicate>
constexpr InputIt find_if_not(InputIt first, InputIt last, UnaryPredicate q)
{
    for (; first != last; ++first) {
        if (!q(*first)) {
            return first;
        }
    }
    return last;
}

template<class ForwardIt, class UnaryPredicate>
constexpr ForwardIt partition(ForwardIt first, ForwardIt last, UnaryPredicate p)
{
    first = KDFoundation::find_if_not(first, last, p);
    if (first == last)
        return first;

    for (ForwardIt i = std::next(first); i != last; ++i) {
        if (p(*i)) {
            KDFoundation::iter_swap(i, first);
            ++first;
        }
    }
    return first;
}

template<class RAIt, class Compare = std::less<>>
constexpr void quick_sort(RAIt first, RAIt last, Compare cmp = Compare{})
{
    auto const N = std::distance(first, last);
    if (N <= 1)
        return;
    auto const pivot = *std::next(first, N / 2);
    auto const middle1 = KDFoundation::partition(first, last, [=](auto const &elem) {
        return cmp(elem, pivot);
    });
    auto const middle2 = KDFoundation::partition(middle1, last, [=](auto const &elem) {
        return !cmp(pivot, elem);
    });
    quick_sort(first, middle1, cmp); // assert(std::is_sorted(first, middle1, cmp));
    quick_sort(middle2, last, cmp); // assert(std::is_sorted(middle2, last, cmp));
}

template<typename Range>
constexpr auto sort(Range &&range)
{
    quick_sort(std::begin(range), std::end(range));
    return range;
}

} // namespace KDFoundation
