// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Cocktail_sort

#pragma once

#include "Algorithms/Core/GeneratorSort.hpp"

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class CocktailSort final : public GeneratorSortOf<CocktailSort>
{
public:
    static constexpr AlgorithmMeta kMeta{
        "Cocktail Sort",
        "O(n²)",
        "Bidirectional variation of Bubble Sort."
    };

protected:
    StepGenerator run() override;
};

} // namespace Sortly
