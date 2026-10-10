// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

namespace Sortly {

struct SortOperation
{
    enum class Type
    {
        Compare,   // Represents a comparison between two elements.
        Swap,      // Represents a swap of two elements.
        Overwrite, // Represents overwriting an element with a new value.
        Pivot,     // A pivot element is selected (like QuickSort)
        Sorted     // An element is marked as sorted or in its final position.
    };

    Type type = Type::Compare;

    int indexA = -1; // Primary index (always applicable).
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

} // namespace Sortly
