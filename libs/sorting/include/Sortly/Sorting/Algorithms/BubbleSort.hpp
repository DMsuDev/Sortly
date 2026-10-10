// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Bubble_sort

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

namespace Sortly {

class BubbleSort final : public SortAlgorithm
{
public:
    void load(std::span<const int> data) override;

    std::optional<SortOperation> step(SortRecorder& recorder) override;

    [[nodiscard]] std::vector<int> getData() const override { return m_Data; }

    [[nodiscard]] std::string_view getName() const override { return "Bubble Sort"; }
    [[nodiscard]] std::string_view getComplexity() const override { return "O(n²)"; }
    [[nodiscard]] std::string_view getDescription() const override
    {
        return "Simple comparison-based sorting algorithm.";
    }
private:
    std::vector<int> m_Data; // The data being sorted.

    size_t m_I = 0; // The current pass index.
    size_t m_J = 0; // The current comparison index within the pass.

    bool m_Finished = false;
};

} // namespace Sortly
