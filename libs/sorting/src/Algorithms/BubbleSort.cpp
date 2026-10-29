// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "BubbleSort.hpp"

namespace Sortly {

// Each pass bubbles the largest remaining element to the end of the unsorted range.
// Every comparison is yielded (and so counted), whether or not it ends in a swap.
// A pass without swaps proves the array is sorted, so the run stops early.
StepGenerator BubbleSort::run()
{
    for (std::size_t end = m_Data.size(); end > 1; --end)
    {
        bool swapped = false;
        for (std::size_t i = 0; i + 1 < end; ++i)
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
    }
}

} // namespace Sortly
