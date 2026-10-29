// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Bubble_sort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class BubbleSort final : public GeneratorSortOf<BubbleSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Bubble Sort",
        "O(n²)",
        "Swaps adjacent out-of-order pairs; stops once a pass makes no swap."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
