// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "SelectionSort.hpp"

namespace Sortly {

StepGenerator SelectionSort::run()
{
    const std::size_t n = m_Data.size();

    for (std::size_t i = 0; i + 1 < n; ++i)
    {
        std::size_t smallest = i;
        for (std::size_t j = i + 1; j < n; ++j)
        {
            co_yield Step::compare(smallest, j);
            if (m_Data[j] < m_Data[smallest])
                smallest = j;
        }

        if (smallest != i)
            co_yield Step::swap(i, smallest);
    }
}

} // namespace Sortly
