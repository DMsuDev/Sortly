// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "InsertionSort.hpp"

namespace Sortly {

// Element `i` slides left, one swap at a time, until the element before it is not larger.
// Everything left of `i` is already sorted.
StepGenerator InsertionSort::run()
{
    const std::size_t n = m_Data.size();

    for (std::size_t i = 1; i < n; ++i)
    {
        for (std::size_t j = i; j > 0; --j)
        {
            co_yield Step::compare(j - 1, j);
            if (m_Data[j - 1] <= m_Data[j])
                break;
            co_yield Step::swap(j - 1, j);
        }
    }
}

} // namespace Sortly
