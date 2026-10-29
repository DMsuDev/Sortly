// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Merge_sort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class MergeSort final : public GeneratorSortOf<MergeSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Merge Sort",
        "O(n log n)",
        "Bottom-up merging of sorted runs through an auxiliary buffer."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
