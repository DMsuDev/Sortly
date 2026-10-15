// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"
#include "Sortly/Sorting/Operations/SortRecorder.hpp"

#include <optional>
#include <span>
#include <string_view>

namespace Sortly {

class SortAlgorithm
{
public:
    virtual ~SortAlgorithm() = default;

    /// @brief Initializes the algorithm with the given data.
    /// Copies the data and resets any internal state, so the algorithm can be reused.
    virtual void load(std::span<const int> data) = 0;

    /// @brief Advances the algorithm by one logical operation.
    ///
    /// Contract: every run emits any number of Compare/Swap/Overwrite/Pivot operations followed by
    /// exactly one Sorted operation (also for empty or single-element input), and
    /// std::nullopt from then on. The algorithm never builds SortOperation values itself:
    /// it goes through the recorder, which also keeps the session's statistics in sync.
    /// @return The operation performed, or `std::nullopt` once the run has completed.
    virtual std::optional<SortOperation> step(SortRecorder& recorder) = 0;

    /// @brief The algorithm's working copy of the data (sorted once the run completes).
    [[nodiscard]] virtual std::span<const int> getData() const = 0;

    [[nodiscard]] virtual std::string_view getName() const = 0;
    [[nodiscard]] virtual std::string_view getComplexity() const = 0;
    [[nodiscard]] virtual std::string_view getDescription() const = 0;
};

} // namespace Sortly
