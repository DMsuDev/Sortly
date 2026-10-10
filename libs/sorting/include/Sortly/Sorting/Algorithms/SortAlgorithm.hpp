// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortOperation.hpp"
#include "Sortly/Sorting/Operations/detail/SortRecorder.hpp"

#include <span>
#include <vector>
#include <optional>
#include <string_view>

namespace Sortly {

class SortAlgorithm
{
public:
    virtual ~SortAlgorithm() = default;

    /// @brief Initializes the algorithm with the given data.
    /// Set up any necessary internal state and prepare for sorting.
    virtual void load(std::span<const int> data) = 0;

    /// @brief Advances the algorithm by one logical operation.
    /// The algorithm never builds SortOperation values itself - it always goes
    /// through the recorder, which also keeps the session's statistics in sync.
    /// @return An optional `SortOperation` describing the performed action, or `std::nullopt` if the algorithm has completed.
    virtual std::optional<SortOperation> step(SortRecorder& recorder) = 0;

    [[nodiscard]] virtual std::vector<int> getData() const = 0;

    [[nodiscard]] virtual std::string_view getName() const = 0;
    [[nodiscard]] virtual std::string_view getComplexity() const = 0;
    [[nodiscard]] virtual std::string_view getDescription() const = 0;
};

} // namespace Sortly
