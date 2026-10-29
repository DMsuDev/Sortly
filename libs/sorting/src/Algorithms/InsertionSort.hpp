// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Insertion_sort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class InsertionSort final : public GeneratorSortOf<InsertionSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Insertion Sort",
        "O(n²)",
        "Grows a sorted prefix by sliding each element into its place."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
