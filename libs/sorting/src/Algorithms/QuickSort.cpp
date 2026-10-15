// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "QuickSort.hpp"

#include <utility>
#include <vector>

namespace Sortly {

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

        co_yield Step::pivot(high);
        const int pivot = m_Data[high]; // Stays in place until the final swap below.

        std::size_t boundary = low; // Everything before it is smaller than the pivot.
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

        if (boundary != high)
            co_yield Step::swap(boundary, high);

        // Only ranges with at least two elements are worth another pass.
        if (boundary > low + 1)
            ranges.emplace_back(low, boundary - 1);
        if (boundary + 1 < high)
            ranges.emplace_back(boundary + 1, high);
    }
}

} // namespace Sortly
