// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Heapsort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class HeapSort final : public GeneratorSortOf<HeapSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Heap Sort",
        "O(n log n)",
        "Builds a max-heap, then repeatedly moves its root to the end."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
