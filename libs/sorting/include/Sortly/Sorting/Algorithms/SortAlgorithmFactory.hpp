// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace Sortly {

enum class SortAlgorithmType
{
    BubbleSort = 0,
    CocktailSort,
    SelectionSort,
    InsertionSort,
    QuickSort,
    MergeSort,
    HeapSort,
    ShellSort
};

struct AlgorithmInfo
{
    SortAlgorithmType type;
    std::string_view  name;   // string literal: null-terminated, safe for .data()
    std::unique_ptr<SortAlgorithm> (*create)();
};

/// @brief Creates algorithms by type. The concrete classes are private to the library
/// (src/Algorithms), so adding one never changes a public header: implement it, then
/// register it with one line in SortAlgorithmFactory.cpp.
class SortAlgorithmFactory
{
public:
    /// @brief Every registered (implemented) algorithm.
    [[nodiscard]] static std::span<const AlgorithmInfo> all();

    /// @brief Registration entry for @p type, or nullptr if it is not implemented. Silent.
    [[nodiscard]] static const AlgorithmInfo* find(SortAlgorithmType type);

    /// @brief New instance of @p type, or nullptr (and an error log) if it is not implemented.
    [[nodiscard]] static std::unique_ptr<SortAlgorithm> create(SortAlgorithmType type);

    [[nodiscard]] static std::string_view toString(SortAlgorithmType type);

    [[nodiscard]] static std::vector<SortAlgorithmType> getAvailableAlgorithms();
};

} // namespace Sortly
