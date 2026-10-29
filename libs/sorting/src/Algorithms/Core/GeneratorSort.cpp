// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "GeneratorSort.hpp"

#include <stdexcept>
#include <string>

namespace Sortly {

namespace {

void requireInRange(std::size_t index, std::size_t size, const char* which)
{
    if (index >= size)
        throw std::out_of_range(std::string("GeneratorSort: step index ") + which + " = " + std::to_string(index) +
                                " is out of range (size " + std::to_string(size) + ")");
}

} // namespace

void GeneratorSort::load(std::span<const int> data)
{
    m_Data.assign(data.begin(), data.end());
    m_Aux.assign(data.begin(), data.end());
    m_Finished  = false;
    m_Generator = run(); // Lazy: nothing executes until the first step().
}

void GeneratorSort::validate(const Step& step) const
{
    switch (step.kind)
    {
        case Step::Kind::Compare:
        {
            const std::size_t size = step.aux ? m_Aux.size() : m_Data.size();
            requireInRange(step.a, size, "a");
            requireInRange(step.b, size, "b");
            break;
        }
        case Step::Kind::Swap:
            requireInRange(step.a, m_Data.size(), "a");
            requireInRange(step.b, m_Data.size(), "b");
            break;
        case Step::Kind::Overwrite:
        case Step::Kind::Pivot:
            requireInRange(step.a, m_Data.size(), "a");
            break;
        case Step::Kind::ReleasePivot:
            break;
    }
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
    validate(s);

    switch (s.kind)
    {
        case Step::Kind::Compare:
            return recorder.compare(s.aux ? std::span<const int>(m_Aux) : std::span<const int>(m_Data), s.a, s.b);
        case Step::Kind::Swap:         return recorder.swap(m_Data, s.a, s.b);
        case Step::Kind::Overwrite:    return recorder.overwrite(m_Data, s.a, s.value);
        case Step::Kind::Pivot:        return recorder.pivot(s.a, m_Data[s.a]);
        case Step::Kind::ReleasePivot: return recorder.releasePivot();
    }
    return std::nullopt;
}

} // namespace Sortly
