// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Bubble_sort

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class BubbleSort final : public SortAlgorithm
{
public:
    // Single source of truth for the metadata; the factory table reads kName too.
    static constexpr std::string_view kName        = "Bubble Sort";
    static constexpr std::string_view kComplexity  = "O(n²)";
    static constexpr std::string_view kDescription = "Simple comparison-based sorting algorithm.";

    void load(std::span<const int> data) override;

    std::optional<SortOperation> step(SortRecorder& recorder) override;

    [[nodiscard]] std::span<const int> getData() const override { return m_Data; }

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

private:
    std::vector<int> m_Data; // The data being sorted.

    std::size_t m_I = 0; // The current pass index.
    std::size_t m_J = 0; // The current comparison index within the pass.

    bool m_Finished = false;
};

} // namespace Sortly
