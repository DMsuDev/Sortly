// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Algorithms/BubbleSort.hpp"
#include <algorithm>

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

    if ((m_I >= m_Data.size() - 1) || m_Data.empty() || m_Data.size() == 1)
    {
        m_Finished = true;
        return recorder.sorted();
    }

    int indexA = static_cast<int>(m_J);
    int indexB = static_cast<int>(m_J + 1);
    int valueA = m_Data[m_J];
    int valueB = m_Data[m_J + 1];

    SortOperation op;

    if (valueA > valueB)
    {
        std::swap(m_Data[m_J], m_Data[m_J + 1]);
        op = recorder.swap(indexA, indexB, valueA, valueB);
    }
    else
    {
        op = recorder.compare(indexA, indexB, valueA, valueB);
    }

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
