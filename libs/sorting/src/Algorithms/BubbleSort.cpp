// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "BubbleSort.hpp"

namespace Sortly {

void BubbleSort::load(std::span<const int> data)
{
    m_Data.assign(data.begin(), data.end());

    m_I = 0;
    m_J = 0;
    m_Finished = false;
}

std::optional<SortOperation> BubbleSort::step(SortRecorder& recorder)
{
    if (m_Finished)
        return std::nullopt;

    // Fewer than two elements, or every pass done: nothing left to compare.
    // (size() < 2 is checked first so size() - 1 can never underflow.)
    if (m_Data.size() < 2 || m_I >= m_Data.size() - 1)
    {
        m_Finished = true;
        return recorder.sorted();
    }

    const std::size_t a = m_J;
    const std::size_t b = m_J + 1;

    // The recorder reads (and swaps) the buffer itself, so the recorded values can't go stale.
    const auto op = (m_Data[a] > m_Data[b]) ? recorder.swap(m_Data, a, b)
                                            : recorder.compare(m_Data, a, b);

    ++m_J;

    // If we've reached the end of the current pass, reset J and move to the next pass.
    if (m_J >= m_Data.size() - m_I - 1)
    {
        m_J = 0;
        ++m_I;
    }

    return op;
}

} // namespace Sortly
