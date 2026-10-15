// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "HeapSort.hpp"

namespace Sortly {

StepGenerator HeapSort::run()
{
    std::size_t heapEnd = m_Data.size(); // Exclusive end of the heap.
    std::size_t start   = heapEnd / 2;   // Next node to sift down while the heap is being built.

    // One loop for both phases: first `start` walks down to the root (heapify), then each pass
    // moves the root behind the heap and shrinks it.
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
