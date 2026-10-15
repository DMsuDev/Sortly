// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Shellsort

#pragma once

#include "GeneratorSort.hpp"

#include <string_view>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class ShellSort final : public GeneratorSort
{
public:
    static constexpr std::string_view kName        = "Shell Sort";
    static constexpr std::string_view kComplexity  = "O(n log² n)";
    static constexpr std::string_view kDescription = "Insertion sort over shrinking gaps, so distant elements move early.";

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

protected:
    StepGenerator run() override;
};

} // namespace Sortly
