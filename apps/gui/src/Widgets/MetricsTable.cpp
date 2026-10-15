// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/MetricsTable.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <cstdio>

namespace Sortly::ui {

void drawMetricsTable(const SortStatistics* stats)
{
    struct Metric
    {
        const char* icon;
        const char* label;
        std::string value;
        const char* unit;
        ImVec4      color;
    };

    const auto count = [&](std::uint64_t SortStatistics::* member) {
        return stats ? formatCount(stats->*member) : std::string("-");
    };

    std::string compute = "-";
    if (stats)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.2f", stats->computeTimeMs);
        compute = buffer;
    }

    const Metric metrics[] = {
        { ICON_COMPARE,  "COMPARISONS", count(&SortStatistics::comparisons),    nullptr, colors::Info    },
        { ICON_SWAP,     "SWAPS",       count(&SortStatistics::swaps),          nullptr, colors::Danger  },
        { ICON_LIST,     "OPERATIONS",  count(&SortStatistics::operationCount), nullptr, colors::Accent  },
        { ICON_ACTIVITY, "READS",       count(&SortStatistics::reads),          nullptr, colors::Violet  },
        { ICON_CPU,      "WRITES",      count(&SortStatistics::writes),         nullptr, colors::Warning },
        { ICON_TIMER,    "COMPUTE",     compute, stats ? "ms" : nullptr,                 colors::Success },
    };

    constexpr int   kMetricCount = static_cast<int>(sizeof(metrics) / sizeof(metrics[0]));
    constexpr float kCardHeight  = 92.0f;
    constexpr float kMinCardWidth = 170.0f;

    const float gap   = ImGui::GetStyle().ItemSpacing.x;
    const float avail = ImGui::GetContentRegionAvail().x;
    // Only divisors of the metric count, so the last row is never left with an orphan card.
    int columns = 1;
    for (const int candidate : { 6, 3, 2 })
    {
        if (kMetricCount % candidate == 0 &&
            avail >= candidate * kMinCardWidth + gap * static_cast<float>(candidate - 1))
        {
            columns = candidate;
            break;
        }
    }
    const float cardWidth = (avail - gap * static_cast<float>(columns - 1)) / static_cast<float>(columns);

    for (int i = 0; i < kMetricCount; ++i)
    {
        if (i % columns != 0)
            ImGui::SameLine();
        ImGui::PushID(i);
        statCard(metrics[i].icon, metrics[i].label, metrics[i].value, metrics[i].unit,
                 metrics[i].color, ImVec2(cardWidth, kCardHeight));
        ImGui::PopID();
    }
}

} // namespace Sortly::ui
