// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Algorithms/SortAlgorithm.hpp"

#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace Sortly {

/// @brief Identifies an algorithm. Values are indices into the registry, so the order here
/// must match the order of the registry in SortAlgorithmFactory.cpp (checked at compile time).
enum class SortAlgorithmType
{
    BubbleSort = 0,
    CocktailSort,
    SelectionSort,
    InsertionSort,
    QuickSort,
    MergeSort,
    HeapSort,
    ShellSort,

    Count // Keep last: number of registered algorithms. Not an algorithm.
};

/// @brief One registry entry. Metadata is available without creating the algorithm.
struct AlgorithmInfo
{
    SortAlgorithmType type;
    std::string_view  name;        // String literal: null-terminated, safe for .data().
    std::string_view  complexity;
    std::string_view  description;
    std::unique_ptr<SortAlgorithm> (*create)();
};

/// @brief Creates algorithms by type. The concrete classes are private to the library
/// (src/Algorithms). Adding one touches this header once (a new enumerator before Count)
/// and SortAlgorithmFactory.cpp once (one registry line).
class SortAlgorithmFactory
{
public:
    /// @brief Every registered algorithm, indexed by SortAlgorithmType.
    [[nodiscard]] static std::span<const AlgorithmInfo> all();

    /// @brief Registry entry for @p type, or nullptr if it is not registered. Silent.
    [[nodiscard]] static const AlgorithmInfo* find(SortAlgorithmType type);

    /// @brief New instance of @p type, or nullptr (and an error log) if it is not registered.
    [[nodiscard]] static std::unique_ptr<SortAlgorithm> create(SortAlgorithmType type);

    [[nodiscard]] static std::string_view toString(SortAlgorithmType type);

    [[nodiscard]] static std::vector<SortAlgorithmType> getAvailableAlgorithms();
};

} // namespace Sortly
