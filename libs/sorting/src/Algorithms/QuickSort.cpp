// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "QuickSort.hpp"

#include <utility>
#include <vector>

namespace Sortly {

// Lomuto partition with a median-of-three pivot.
//
// Pivot protocol (what the UI shows): Step::pivot(high) marks the pivot once it sits at `high`;
// it stays there during the whole partition, then the final swap moves it to its final position
// and Step::releasePivot() clears the mark before the next range starts.
StepGenerator QuickSort::run()
{
    if (m_Data.size() < 2)
        co_return;

    // Explicit stack of [low, high] ranges: a coroutine cannot recurse into itself cheaply.
    std::vector<std::pair<std::size_t, std::size_t>> ranges{ { 0, m_Data.size() - 1 } };

    while (!ranges.empty())
    {
        const auto [low, high] = ranges.back();
        ranges.pop_back();

        // Median of three: order low/mid/high, then move the median to `high` so it can be the
        // pivot. Avoids the O(n^2) case on sorted input.
        if (high - low >= 2)
        {
            const std::size_t mid = low + (high - low) / 2;

            co_yield Step::compare(low, mid);
            if (m_Data[mid] < m_Data[low])
                co_yield Step::swap(low, mid);

            co_yield Step::compare(low, high);
            if (m_Data[high] < m_Data[low])
                co_yield Step::swap(low, high);

            co_yield Step::compare(mid, high);
            if (m_Data[high] < m_Data[mid])
                co_yield Step::swap(mid, high);

            // Now data[low] <= data[mid] <= data[high]; the median sits at mid.
            co_yield Step::swap(mid, high);
        }

        co_yield Step::pivot(high);
        const int pivot = m_Data[high]; // Stays in place until the final swap below.

        std::size_t boundary = low; // Everything in [low, boundary) is smaller than the pivot.
        for (std::size_t j = low; j < high; ++j)
        {
            co_yield Step::compare(j, high);
            if (m_Data[j] < pivot)
            {
                if (boundary != j)
                    co_yield Step::swap(boundary, j);
                ++boundary;
            }
        }

        // The pivot lands in its final position (SortPlayback moves the pivot mark with it).
        if (boundary != high)
            co_yield Step::swap(boundary, high);
        co_yield Step::releasePivot();

        // Right first, so the left range is popped first and the sort sweeps left to right.
        // Only ranges with at least two elements are worth another pass.
        if (boundary + 1 < high)
            ranges.emplace_back(boundary + 1, high);
        if (boundary > low + 1)
            ranges.emplace_back(low, boundary - 1);
    }
}

} // namespace Sortly
