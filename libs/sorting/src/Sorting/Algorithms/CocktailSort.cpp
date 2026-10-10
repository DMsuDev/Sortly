// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Sorting/Algorithms/CocktailSort.hpp"
#include <algorithm>

namespace Sortly {

void CocktailSort::load(std::span<const int> data)
{
    m_Data.assign(data.begin(), data.end());

    m_Direction = CocktailSort::Direction::Forward;
    m_Index = 0;

    m_Start = 0;
    m_End = m_Data.empty() ? 0 : m_Data.size() - 1;

    m_Swapped = false;
    m_Finished = false;

    m_Direction = CocktailSort::Direction::Forward;
}

std::optional<SortOperation> CocktailSort::step(SortRecorder& recorder)
{
    if (m_Finished)
        return std::nullopt;

    if (m_Data.empty())
    {
        m_Finished = true;
        return std::nullopt;
    }

    while (true)
    {
        if (m_Start >= m_End)
        {
            m_Finished = true;
            return recorder.sorted();
        }

        if (m_Direction == CocktailSort::Direction::Forward)
        {
            if (m_Index >= m_End)
            {
                if (!m_Swapped)
                {
                    m_Finished = true;
                    return recorder.sorted();
                }

                m_Direction = CocktailSort::Direction::Backward;
                m_End--;
                m_Index = m_End;
                m_Swapped = false;
                continue;
            }

            int valueA = m_Data[m_Index];
            int valueB = m_Data[m_Index + 1];

            SortOperation op;

            if (valueA > valueB)
            {
                std::swap(m_Data[m_Index], m_Data[m_Index + 1]);
                op = recorder.swap(static_cast<int>(m_Index), static_cast<int>(m_Index + 1), valueA, valueB);
                m_Swapped = true;
            }
            else
            {
                op = recorder.compare(static_cast<int>(m_Index), static_cast<int>(m_Index + 1), valueA, valueB);
            }

            m_Index++;
            return op;
        }
        else // Backward
        {
            if (m_Index <= m_Start)
            {
                if (!m_Swapped)
                {
                    m_Finished = true;
                    return recorder.sorted();
                }

                m_Direction = CocktailSort::Direction::Forward;
                m_Start++;
                m_Index = m_Start;
                m_Swapped = false;
                continue;
            }

            int valueA = m_Data[m_Index - 1];
            int valueB = m_Data[m_Index];

            SortOperation op;

            if (valueA > valueB)
            {
                std::swap(m_Data[m_Index - 1], m_Data[m_Index]);
                op = recorder.swap(static_cast<int>(m_Index - 1), static_cast<int>(m_Index), valueA, valueB);
                m_Swapped = true;
            }
            else
            {
                op = recorder.compare(static_cast<int>(m_Index - 1), static_cast<int>(m_Index), valueA, valueB);
            }

            m_Index--;
            return op;
        }
    }
}

} // namespace Sortly
