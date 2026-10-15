// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Selection_sort

#pragma once

#include "GeneratorSort.hpp"

#include <string_view>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class SelectionSort final : public GeneratorSort
{
public:
    static constexpr std::string_view kName        = "Selection Sort";
    static constexpr std::string_view kComplexity  = "O(n²)";
    static constexpr std::string_view kDescription = "Repeatedly selects the smallest remaining element and moves it into place.";

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

protected:
    StepGenerator run() override;
};

} // namespace Sortly
