// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>

namespace Sortly {

/// @brief One thing an algorithm wants to do next.
///
/// The algorithm only *describes* the step with `co_yield Step::...`; GeneratorSort performs it
/// through the SortRecorder, which keeps data, operations and statistics in sync.
/// Build steps with the factory functions below, never by hand.
struct Step
{
    enum class Kind { Compare, Swap, Overwrite, Pivot, ReleasePivot };

    Kind        kind  = Kind::Compare;
    std::size_t a     = 0;     // First index (all kinds except ReleasePivot).
    std::size_t b     = 0;     // Second index (Compare, Swap).
    int         value = 0;     // Overwrite: the new value.
    bool        aux   = false; // Compare: read the values from the auxiliary buffer.

    /// Compare m_Data[i] with m_Data[j]. Does not change the data.
    static constexpr Step compare(std::size_t i, std::size_t j)    { return { Kind::Compare, i, j, 0, false }; }
    /// Compare m_Aux[i] with m_Aux[j] (merge-style algorithms). Does not change the data.
    static constexpr Step compareAux(std::size_t i, std::size_t j) { return { Kind::Compare, i, j, 0, true }; }
    /// Swap m_Data[i] and m_Data[j].
    static constexpr Step swap(std::size_t i, std::size_t j)       { return { Kind::Swap, i, j, 0, false }; }
    /// Write newValue into m_Data[i].
    static constexpr Step overwrite(std::size_t i, int newValue)   { return { Kind::Overwrite, i, 0, newValue, false }; }
    /// Mark m_Data[i] as the pivot. Does not change the data.
    static constexpr Step pivot(std::size_t i)                     { return { Kind::Pivot, i, 0, 0, false }; }
    /// Clear the pivot mark. Does not change the data.
    static constexpr Step releasePivot()                           { return { Kind::ReleasePivot, 0, 0, 0, false }; }
};

} // namespace Sortly
