// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"
#include "Sortly/Sorting/Metrics/SortStatistics.hpp"

#include <span>

namespace Sortly {

// Builds SortOperation instances on behalf of a SortAlgorithm and updates the
// session's SortStatistics as a byproduct. Algorithms never construct
// SortOperation manually - they call recorder.compare()/swap()/... instead,
// so operation creation and statistics counting can never go out of sync.
class SortRecorder
{
public:
    explicit SortRecorder(SortStatistics& statistics) : m_Statistics(statistics) {}

    [[nodiscard]] SortOperation compare(int indexA, int indexB, int valueA, int valueB);
    [[nodiscard]] SortOperation swap(int indexA, int indexB, int valueA, int valueB);

    /// @brief Overwrites @p buffer[index] with @p newValue, recording both the new and
    /// the previous value. The recorder reads and mutates @p buffer itself so the caller
    /// can never pass a mismatched old value, keeping the resulting SortOperation reversible.
    [[nodiscard]] SortOperation overwrite(std::span<int> buffer, int index, int newValue);

    [[nodiscard]] SortOperation pivot(int index, int value);
    [[nodiscard]] SortOperation sorted();

private:
    SortOperation record(SortOperation op);

    SortStatistics& m_Statistics;
};

} // namespace Sortly
