// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/RunMetrics.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"

#include <cstdio>

namespace Sortly::ui {

const char* metricLabel(Metric metric)
{
    switch (metric)
    {
        case Metric::Comparisons: return "Comparisons";
        case Metric::Swaps:       return "Swaps";
        case Metric::Reads:       return "Reads";
        case Metric::Writes:      return "Writes";
        case Metric::ComputeTime: return "Compute time";
    }
    return "";
}

const char* metricWinLabel(Metric metric)
{
    switch (metric)
    {
        case Metric::Comparisons: return "Fewest comparisons";
        case Metric::Swaps:       return "Fewest swaps";
        case Metric::Reads:       return "Fewest reads";
        case Metric::Writes:      return "Fewest writes";
        case Metric::ComputeTime: return "Shortest compute time";
    }
    return "";
}

double metricValue(const SortStatistics& stats, Metric metric)
{
    switch (metric)
    {
        case Metric::Comparisons: return static_cast<double>(stats.comparisons);
        case Metric::Swaps:       return static_cast<double>(stats.swaps);
        case Metric::Reads:       return static_cast<double>(stats.reads);
        case Metric::Writes:      return static_cast<double>(stats.writes);
        case Metric::ComputeTime: return stats.computeTimeMs;
    }
    return 0.0;
}

std::string formatMetric(const SortStatistics& stats, Metric metric, bool withUnit)
{
    if (metric == Metric::ComputeTime)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), withUnit ? "%.2f ms" : "%.2f", stats.computeTimeMs);
        return buffer;
    }
    return formatCount(static_cast<std::uint64_t>(metricValue(stats, metric)));
}

} // namespace Sortly::ui
