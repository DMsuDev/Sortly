// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

// https://en.wikipedia.org/wiki/Cocktail_sort

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace Sortly {

// Private to the library: users create it through SortAlgorithmFactory.
class CocktailSort final : public SortAlgorithm
{
public:
    static constexpr std::string_view kName        = "Cocktail Sort";
    static constexpr std::string_view kComplexity  = "O(n²)";
    static constexpr std::string_view kDescription = "Bidirectional variation of Bubble Sort.";

    void load(std::span<const int> data) override;

    std::optional<SortOperation> step(SortRecorder& recorder) override;

    [[nodiscard]] std::span<const int> getData() const override { return m_Data; }

    [[nodiscard]] std::string_view getName() const override { return kName; }
    [[nodiscard]] std::string_view getComplexity() const override { return kComplexity; }
    [[nodiscard]] std::string_view getDescription() const override { return kDescription; }

private:
    enum class Direction
    {
        Forward,
        Backward
    };

    /// Compares data[left] and data[left + 1], swapping them if out of order (and flagging the pass).
    SortOperation compareOrSwap(SortRecorder& recorder, std::size_t left);

    /// Marks the run as finished and returns its terminal Sorted operation.
    SortOperation finish(SortRecorder& recorder);

    std::vector<int> m_Data; // The data being sorted.

    bool m_Swapped = false; // Whether any swaps were made in the current pass.
    bool m_Finished = false;

    std::size_t m_Start = 0; // The starting index of the unsorted portion (moves rightward).
    std::size_t m_End = 0;   // The ending index of the unsorted portion (moves leftward).
    std::size_t m_Index = 0; // The current comparison index within the pass.

    Direction m_Direction = Direction::Forward;
};

} // namespace Sortly
