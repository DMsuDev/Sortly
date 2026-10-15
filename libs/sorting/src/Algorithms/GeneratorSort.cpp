// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "GeneratorSort.hpp"

namespace Sortly {

void GeneratorSort::load(std::span<const int> data)
{
    m_Data.assign(data.begin(), data.end());
    m_Aux.assign(data.begin(), data.end());
    m_Finished  = false;
    m_Generator = run(); // Lazy: nothing executes until the first step().
}

std::optional<SortOperation> GeneratorSort::step(SortRecorder& recorder)
{
    if (m_Finished)
        return std::nullopt;

    if (!m_Generator.next())
    {
        m_Finished = true;
        return recorder.sorted();
    }

    const Step& s = m_Generator.value();
    switch (s.kind)
    {
        case Step::Kind::Compare:   return recorder.compare(s.aux ? std::span<const int>(m_Aux) : std::span<const int>(m_Data), s.a, s.b);
        case Step::Kind::Swap:      return recorder.swap(m_Data, s.a, s.b);
        case Step::Kind::Overwrite: return recorder.overwrite(m_Data, s.a, s.value);
        case Step::Kind::Pivot:     return recorder.pivot(s.a, m_Data[s.a]);
    }
    return std::nullopt;
}

} // namespace Sortly
