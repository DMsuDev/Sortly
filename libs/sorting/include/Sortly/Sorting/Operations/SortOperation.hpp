// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <string_view>

namespace Sortly {

struct SortOperation
{
    enum class Type
    {
        Compare,   // Represents a comparison between two elements.
        Swap,      // Represents a swap of two elements.
        Overwrite, // Represents overwriting an element with a new value.
        Pivot,     // A pivot element is selected (like QuickSort)
        Sorted     // The run is finished (indexA == -1), or one element reached its final position (indexA >= 0, reserved).
    };

    Type type = Type::Compare;

    int indexA = -1; // Primary index (-1 for Sorted when it marks the end of the whole run).
    int indexB = -1; // Secondary index (-1 if not applicable: Overwrite, Pivot, Sorted).

    // The meaning of valueA/valueB depends on `type` - there is no single rule for both:
    //
    //   Compare / Swap : valueA = value that was at indexA, valueB = value that was at indexB
    //                    (both captured BEFORE the swap happens, so playback/undo can rely on them).
    //   Overwrite      : valueA = the NEW value written to indexA.
    //                    valueB = the OLD value it replaced, captured by SortRecorder::overwrite()
    //                    itself (never by the algorithm). This is what lets SortPlayback undo an
    //                    Overwrite in O(1) - assign valueB back - instead of re-simulating the algorithm.
    //   Pivot          : valueA = the pivot's value. valueB is unused.
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
