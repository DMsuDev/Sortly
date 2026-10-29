// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Shellsort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class ShellSort final : public GeneratorSortOf<ShellSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Shell Sort",
        "O(n log² n)",
        "Insertion sort over shrinking gaps, so distant elements move early."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
