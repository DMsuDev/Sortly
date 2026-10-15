// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstddef>

namespace Sortly {

// Pure value object describing how a DataSet should be generated.
// Holds only configuration data, no generation logic - fully testable
// and serializable on its own.
struct DataSetConfig
{
    int count    = 128;
    int minCount = 2;
    int maxCount = 4096;
    int maxValue = 500;

    // Above this element count, O(n^2) algorithms (BubbleSort, CocktailSort, ...) can generate
    // millions of operations. SortSession uses this as the hard bail-out threshold for the
    // blocking (unlimited-budget) computeOperations() overload; the UI can also read it to warn
    // the user before a run is even started.
    static constexpr std::size_t kQuadraticSafeLimit = 4096;

    DataSetConfig() = default;

    DataSetConfig(int cnt, int minCnt, int maxCnt, int maxVal)
        : count(cnt), minCount(minCnt), maxCount(maxCnt), maxValue(maxVal) {}

    [[nodiscard]] constexpr bool operator==(const DataSetConfig&) const noexcept = default;

    /// @brief Element count after clamping to [minCount, maxCount].
    /// Safe for an inverted range (maxCount < minCount): minCount wins, instead of the
    /// undefined behavior std::clamp has when lo > hi.
    [[nodiscard]] constexpr int effectiveCount() const noexcept
    {
        return std::clamp(count, minCount, std::max(minCount, maxCount));
    }

    /// @brief Largest generated value; at least 1 so the range [1, maxValue] is always valid.
    [[nodiscard]] constexpr int effectiveMaxValue() const noexcept
    {
        return std::max(maxValue, 1);
    }
};

} // namespace Sortly
