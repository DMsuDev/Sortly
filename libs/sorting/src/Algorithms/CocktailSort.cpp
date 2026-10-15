// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "CocktailSort.hpp"

namespace Sortly {

void CocktailSort::load(std::span<const int> data)
{
    m_Data.assign(data.begin(), data.end());

    m_Direction = Direction::Forward;
    m_Index = 0;

    m_Start = 0;
    m_End = m_Data.empty() ? 0 : m_Data.size() - 1;

    m_Swapped = false;
    m_Finished = false;
}

SortOperation CocktailSort::compareOrSwap(SortRecorder& recorder, std::size_t left)
{
    const std::size_t right = left + 1;

    if (m_Data[left] > m_Data[right])
    {
        m_Swapped = true;
        return recorder.swap(m_Data, left, right);
    }

    return recorder.compare(m_Data, left, right);
}

SortOperation CocktailSort::finish(SortRecorder& recorder)
{
    m_Finished = true;
    return recorder.sorted();
}

std::optional<SortOperation> CocktailSort::step(SortRecorder& recorder)
{
    if (m_Finished)
        return std::nullopt;

    // Pass boundaries don't produce an operation, so keep going until one does.
    while (true)
    {
        // Covers empty and single-element input too (m_End is 0 then).
        if (m_Start >= m_End)
            return finish(recorder);

        if (m_Direction == Direction::Forward)
        {
            if (m_Index >= m_End)
            {
                if (!m_Swapped)
                    return finish(recorder);

                m_Direction = Direction::Backward;
                --m_End; // The largest remaining element is in place.
                m_Index = m_End;
                m_Swapped = false;
                continue;
            }

            const auto op = compareOrSwap(recorder, m_Index);
            ++m_Index;
            return op;
        }

        // Backward
        if (m_Index <= m_Start)
        {
            if (!m_Swapped)
                return finish(recorder);

            m_Direction = Direction::Forward;
            ++m_Start; // The smallest remaining element is in place.
            m_Index = m_Start;
            m_Swapped = false;
            continue;
        }

        const auto op = compareOrSwap(recorder, m_Index - 1);
        --m_Index;
        return op;
    }
}

} // namespace Sortly
