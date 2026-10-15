// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Quicksort

#pragma once

#include "GeneratorSort.hpp"

#include <string_view>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class QuickSort final : public GeneratorSort
{
public:
    static constexpr std::string_view kName        = "Quick Sort";
    static constexpr std::string_view kComplexity  = "O(n log n)";
    static constexpr std::string_view kDescription = "Partitions around a pivot (Lomuto scheme) and sorts each side.";

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

protected:
    StepGenerator run() override;
};

} // namespace Sortly
