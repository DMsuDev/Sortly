// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Merge_sort

#pragma once

#include "GeneratorSort.hpp"

#include <string_view>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class MergeSort final : public GeneratorSort
{
public:
    static constexpr std::string_view kName        = "Merge Sort";
    static constexpr std::string_view kComplexity  = "O(n log n)";
    static constexpr std::string_view kDescription = "Bottom-up merging of sorted runs through an auxiliary buffer.";

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

protected:
    StepGenerator run() override;
};

} // namespace Sortly
