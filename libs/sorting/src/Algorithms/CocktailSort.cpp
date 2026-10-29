// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "CocktailSort.hpp"

namespace Sortly {

// Bubble Sort in both directions: a forward pass moves the largest element to `end`,
// a backward pass moves the smallest to `start`, and the unsorted range shrinks from both sides.
// Stops as soon as a pass makes no swap.
StepGenerator CocktailSort::run()
{
    if (m_Data.size() < 2)
        co_return;

    std::size_t start = 0;
    std::size_t end   = m_Data.size() - 1;

    while (start < end)
    {
        bool swapped = false;

        for (std::size_t i = start; i < end; ++i)
        {
            co_yield Step::compare(i, i + 1);
            if (m_Data[i] > m_Data[i + 1])
            {
                co_yield Step::swap(i, i + 1);
                swapped = true;
            }
        }
        if (!swapped)
            break;
        --end; // The largest remaining element is in place (end >= 1 here, so no underflow).

        swapped = false;
        for (std::size_t i = end; i > start; --i)
        {
            co_yield Step::compare(i - 1, i);
            if (m_Data[i - 1] > m_Data[i])
            {
                co_yield Step::swap(i - 1, i);
                swapped = true;
            }
        }
        if (!swapped)
            break;
        ++start; // The smallest remaining element is in place.
    }
}

} // namespace Sortly
