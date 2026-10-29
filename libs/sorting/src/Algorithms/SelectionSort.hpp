// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Selection_sort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class SelectionSort final : public GeneratorSortOf<SelectionSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Selection Sort",
        "O(n²)",
        "Repeatedly selects the smallest remaining element and moves it into place."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
