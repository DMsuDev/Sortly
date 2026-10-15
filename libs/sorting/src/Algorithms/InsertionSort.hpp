// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Insertion_sort

#pragma once

#include "GeneratorSort.hpp"

#include <string_view>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class InsertionSort final : public GeneratorSort
{
public:
    static constexpr std::string_view kName        = "Insertion Sort";
    static constexpr std::string_view kComplexity  = "O(n²)";
    static constexpr std::string_view kDescription = "Grows a sorted prefix by sliding each element into its place.";

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

protected:
    StepGenerator run() override;
};

} // namespace Sortly
