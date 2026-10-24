// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"
#include "Sortly/Sorting/Operations/SortStatistics.hpp"

#include <cstddef>
#include <span>

namespace Sortly {

/// @brief Builds SortOperation instances on behalf of a SortAlgorithm and updates the
/// session's SortStatistics as a byproduct.
///
/// Algorithms never construct SortOperation manually, and never copy values into them:
/// the recorder reads (and, for swap/overwrite, mutates) the buffer itself. That way the
/// recorded values can never disagree with what really happened, which is what keeps every
/// operation reversible by SortPlayback.
class SortRecorder
{
public:
    explicit SortRecorder(SortStatistics& statistics) : m_Statistics(statistics) {}

    /// @brief Records a comparison between buffer[i] and buffer[j]. Does not modify the buffer.
    [[nodiscard]] SortOperation compare(std::span<const int> buffer, std::size_t i, std::size_t j);

    /// @brief Swaps buffer[i] and buffer[j] and records the values they held before the swap.
    [[nodiscard]] SortOperation swap(std::span<int> buffer, std::size_t i, std::size_t j);

    /// @brief Overwrites buffer[index] with @p newValue, recording both the new and the previous value.
    [[nodiscard]] SortOperation overwrite(std::span<int> buffer, std::size_t index, int newValue);

    /// @brief Marks buffer[index] as the pivot, recording the value it holds. Does not modify the
    /// buffer. The mark lasts until the next pivot()/clearPivot() or the end of the run.
    [[nodiscard]] SortOperation pivot(std::span<const int> buffer, std::size_t index);

    /// @brief Clears the pivot mark. Call it as soon as the pivot is done with (e.g. when its
    /// partition ends), so the mark never points at an element that is no longer the pivot.
    [[nodiscard]] SortOperation clearPivot();

    /// @brief Marks the end of the run. Every algorithm emits exactly one, as its last operation.
    [[nodiscard]] SortOperation sorted();

private:
    SortOperation record(SortOperation op);

    SortStatistics& m_Statistics;
};

} // namespace Sortly
