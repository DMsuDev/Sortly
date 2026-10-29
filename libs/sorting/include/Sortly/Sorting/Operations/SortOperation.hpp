// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

namespace Sortly {

/// @brief One recorded event of a run. The full list of operations is enough to replay the
/// run forward and backward without running the algorithm again.
struct SortOperation
{
    enum class Type
    {
        Compare,   // Two elements were compared. The data does not change.
        Swap,      // Two elements exchanged places.
        Overwrite, // One slot received a new value (merge-style writes).
        Pivot,     // indexA >= 0: that element becomes the pivot. indexA == -1: the pivot is released.
                   // The data does not change; SortPlayback tracks the pivot (see getPivotIndex()).
        Sorted     // The run is finished (indexA == -1), or one element reached its final position
                   // (indexA >= 0, reserved).
    };

    Type type = Type::Compare;

    int indexA = -1; // Primary index (-1 when not applicable).
    int indexB = -1; // Secondary index (-1 when not applicable).

    // The meaning of valueA/valueB depends on `type`:
    //
    //   Compare / Swap : valueA = value that was at indexA, valueB = value that was at indexB
    //                    (captured BEFORE a swap happens, so playback/undo can rely on them).
    //   Overwrite      : valueA = the NEW value written to indexA.
    //                    valueB = the OLD value it replaced, captured by SortRecorder itself
    //                    (never by the algorithm). This lets SortPlayback undo it in O(1).
    //   Pivot          : valueA = the pivot's value (unused when releasing). valueB is unused.
    //   Sorted         : both are unused.
    int valueA = 0;
    int valueB = 0;
};

/// @brief Human-readable name of an operation type (for logs and UI labels).
[[nodiscard]] constexpr std::string_view toString(SortOperation::Type type) noexcept
{
    switch (type)
    {
        case SortOperation::Type::Compare:   return "Compare";
        case SortOperation::Type::Swap:      return "Swap";
        case SortOperation::Type::Overwrite: return "Overwrite";
        case SortOperation::Type::Pivot:     return "Pivot";
        case SortOperation::Type::Sorted:    return "Sorted";
    }
    return "Unknown";
}

} // namespace Sortly
