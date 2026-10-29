// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "MergeSort.hpp"

#include <algorithm>

namespace Sortly {

// Bottom-up: merge runs of width 1, then 2, 4, ... until one run covers the array.
// Each merge reads from a snapshot in m_Aux while writing the result back into m_Data.
// Note: the snapshot copy itself is not yielded, so it is not counted as reads/writes.
StepGenerator MergeSort::run()
{
    const std::size_t n = m_Data.size();

    for (std::size_t width = 1; width < n; width *= 2)
    {
        for (std::size_t low = 0; low + width < n; low += 2 * width)
        {
            const std::size_t mid  = low + width;
            const std::size_t high = std::min(low + 2 * width, n);

            std::copy(m_Data.begin() + static_cast<std::ptrdiff_t>(low), m_Data.begin() + static_cast<std::ptrdiff_t>(high),
                      m_Aux.begin() + static_cast<std::ptrdiff_t>(low));

            std::size_t left = low, right = mid, out = low;
            while (left < mid && right < high)
            {
                co_yield Step::compareAux(left, right);
                if (m_Aux[left] <= m_Aux[right])
                    co_yield Step::overwrite(out++, m_Aux[left++]);
                else
                    co_yield Step::overwrite(out++, m_Aux[right++]);
            }
            while (left < mid)
                co_yield Step::overwrite(out++, m_Aux[left++]);
            while (right < high)
                co_yield Step::overwrite(out++, m_Aux[right++]);
        }
    }
}

} // namespace Sortly
