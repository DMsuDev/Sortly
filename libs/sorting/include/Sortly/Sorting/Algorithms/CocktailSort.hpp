// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Cocktail_sort

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

namespace Sortly {

class CocktailSort final : public SortAlgorithm
{
public:
    void load(std::span<const int> data) override;

    std::optional<SortOperation> step(SortRecorder& recorder) override;

    [[nodiscard]] std::vector<int> getData() const override { return m_Data; }

    [[nodiscard]] std::string_view getName() const override { return "Cocktail Sort"; }
    [[nodiscard]] std::string_view getComplexity() const override { return "O(n²)"; }
    [[nodiscard]] std::string_view getDescription() const override
    {
        return "Bidirectional variation of Bubble Sort.";
    }
private:

    enum class Direction
    {
        Forward,
        Backward
    };

    std::vector<int> m_Data; // The data being sorted.

    bool m_Swapped = false; // Whether any swaps were made in the current pass.
    bool m_Finished = false;

    size_t m_Start = 0; // The starting index of the unsorted portion (moves rightward).
    size_t m_End = 0;   // The ending index of the unsorted portion (moves leftward).
    size_t m_Index = 0; // The current comparison index within the pass.

    Direction m_Direction = Direction::Forward;
};

} // namespace Sortly
