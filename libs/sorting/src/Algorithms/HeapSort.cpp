// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "HeapSort.hpp"

namespace Sortly {

// One loop for both phases:
//  1. Heapify: `start` walks from the last parent down to the root, sifting each node down.
//  2. Sort: the root (the maximum) is swapped behind the heap, which then shrinks by one.
// `heapEnd` is the exclusive end of the heap; everything from it onwards is already sorted.
StepGenerator HeapSort::run()
{
    std::size_t heapEnd = m_Data.size();
    std::size_t start   = heapEnd / 2;

    while (heapEnd > 1)
    {
        if (start > 0)
        {
            --start;
        }
        else
        {
            --heapEnd;
            co_yield Step::swap(0, heapEnd);
        }

        // Sift `root` down until both children are smaller.
        std::size_t root = start;
        while (2 * root + 1 < heapEnd)
        {
            std::size_t child = 2 * root + 1;
            if (child + 1 < heapEnd)
            {
                co_yield Step::compare(child, child + 1);
                if (m_Data[child] < m_Data[child + 1])
                    ++child;
            }

            co_yield Step::compare(root, child);
            if (m_Data[root] >= m_Data[child])
                break;

            co_yield Step::swap(root, child);
            root = child;
        }
    }
}

} // namespace Sortly
