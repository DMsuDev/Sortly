// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"
#include "Sortly/Sorting/Operations/SortRecorder.hpp"

#include <optional>
#include <span>
#include <string_view>

namespace Sortly {

/// @brief Static description of an algorithm.
/// Every concrete algorithm declares one as `kMeta`; the factory reads it, so the UI can list
/// algorithms (name, complexity, description) without creating an instance.
struct AlgorithmMeta
{
    std::string_view name;        // Display name, e.g. "Merge Sort".
    std::string_view complexity;  // Average-case time, e.g. "O(n log n)".
    std::string_view description; // One short sentence.
};

/// @brief Interface of a sorting algorithm that runs one logical operation at a time.
///
/// Most algorithms should not implement this directly: derive from GeneratorSortOf (private,
/// src/Algorithms/Core) and write the algorithm as straight-line code that yields Steps.
class SortAlgorithm
{
public:
    virtual ~SortAlgorithm() = default;

    /// @brief Copies @p data as the working buffer and resets all internal state,
    /// so one instance can be reused for several runs.
    virtual void load(std::span<const int> data) = 0;

    /// @brief Advances the algorithm by one logical operation.
    ///
    /// Contract: a run emits any number of Compare/Swap/Overwrite/Pivot operations followed by
    /// exactly one Sorted operation (also for empty or single-element input), then
    /// std::nullopt forever. Operations are built only through @p recorder, which also keeps
    /// the session's statistics in sync.
    /// @return The operation performed, or std::nullopt once the run has completed.
    virtual std::optional<SortOperation> step(SortRecorder& recorder) = 0;

    /// @brief The working copy of the data (sorted once the run completes).
    [[nodiscard]] virtual std::span<const int> getData() const = 0;

    [[nodiscard]] virtual std::string_view getName() const = 0;
    [[nodiscard]] virtual std::string_view getComplexity() const = 0;
    [[nodiscard]] virtual std::string_view getDescription() const = 0;
};

} // namespace Sortly
