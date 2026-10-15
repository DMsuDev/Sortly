// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "ShellSort.hpp"

namespace Sortly {

StepGenerator ShellSort::run()
{
    const std::size_t n = m_Data.size();

    for (std::size_t gap = n / 2; gap > 0; gap /= 2)
    {
        for (std::size_t i = gap; i < n; ++i)
        {
            for (std::size_t j = i; j >= gap; j -= gap)
            {
                co_yield Step::compare(j - gap, j);
                if (m_Data[j - gap] <= m_Data[j])
                    break;
                co_yield Step::swap(j - gap, j);
            }
        }
    }
}

} // namespace Sortly
