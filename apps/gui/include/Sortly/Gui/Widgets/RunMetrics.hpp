// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#pragma once

#include "Sortly/Sorting/Operations/SortStatistics.hpp"

#include <array>
#include <string>

namespace Sortly::ui {

/// @brief The measurements compared between algorithms. Lower is better for all of them.
enum class Metric
{
    Comparisons,
    Swaps,
    Reads,
    Writes,
    ComputeTime
};

inline constexpr std::array<Metric, 5> kMetrics = {
    Metric::Comparisons, Metric::Swaps, Metric::Reads, Metric::Writes, Metric::ComputeTime
};

[[nodiscard]] const char* metricLabel(Metric metric);

/// @brief Wording used when an algorithm wins this metric, e.g. "Fewest comparisons".
[[nodiscard]] const char* metricWinLabel(Metric metric);

[[nodiscard]] double metricValue(const SortStatistics& stats, Metric metric);

/// @brief Display text: thousands-separated counts, milliseconds with two decimals.
/// @param withUnit Append " ms" to the compute time (tables put the unit in the row label).
[[nodiscard]] std::string formatMetric(const SortStatistics& stats, Metric metric, bool withUnit = true);

} // namespace Sortly::ui
