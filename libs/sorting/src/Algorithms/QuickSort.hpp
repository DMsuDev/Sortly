// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Quicksort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class QuickSort final : public GeneratorSortOf<QuickSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Quick Sort",
        "O(n log n)",
        "Partitions around a pivot (Lomuto scheme) and sorts each side."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
