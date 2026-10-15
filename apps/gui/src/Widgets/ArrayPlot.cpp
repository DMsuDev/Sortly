// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>
#include <implot.h>

#include <algorithm>

namespace Sortly::ui {

namespace {

ImVec4 operationColor(SortOperation::Type type)
{
    switch (type)
    {
        case SortOperation::Type::Compare:   return colors::Warning;
        case SortOperation::Type::Swap:
        case SortOperation::Type::Overwrite: return colors::Danger;
        case SortOperation::Type::Pivot:     return colors::Violet;
        case SortOperation::Type::Sorted:    return colors::Success;
    }
    return colors::Accent;
}

// Copy of the data keeping only the given indices (zero elsewhere), plotted over the base bars.
std::vector<int> isolate(const std::vector<int>& data, int a, int b)
{
    std::vector<int> out(data.size(), 0);
    for (const int index : { a, b })
        if (index >= 0 && static_cast<std::size_t>(index) < data.size())
            out[static_cast<std::size_t>(index)] = data[static_cast<std::size_t>(index)];
    return out;
}

} // namespace

void drawArrayPlot(const std::vector<int>& data, const ArrayHighlight& highlight, float height)
{
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,   ImVec4(0, 0, 0, 0));
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,    colors::withAlpha(colors::Clear, 0.55f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, ImVec4(0.17f, 0.20f, 0.25f, 0.8f));
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,  ImVec4(1, 1, 1, 0.06f));
    ImPlot::PushStyleColor(ImPlotCol_AxisText,  colors::Muted);
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(14.0f, 12.0f));

    if (ImPlot::BeginPlot("##ArrayPlot", ImVec2(-1.0f, height),
                          ImPlotFlags_NoLegend | ImPlotFlags_NoMenus | ImPlotFlags_NoMouseText))
    {
        ImPlot::SetupAxes("Index", "Value", ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_AutoFit);

        if (!data.empty())
        {
            const int maxValue = std::max(1, *std::max_element(data.begin(), data.end()));
            ImPlot::SetupAxesLimits(-0.5, static_cast<double>(data.size()) - 0.5,
                                    0.0, static_cast<double>(maxValue) * 1.1, ImPlotCond_Always);

            const ImVec4 base = highlight.sorted ? colors::withAlpha(colors::Success, 0.88f) : colors::Bar;
            ImPlot::PlotBars("Values", data.data(), static_cast<int>(data.size()),
                             0.78, 0.0, ImPlotSpec{ ImPlotProp_FillColor, base });

            if (const SortOperation* op = highlight.operation; op && !highlight.sorted)
            {
                const auto marked = isolate(data, op->indexA, op->indexB);
                ImPlot::PlotBars("Active", marked.data(), static_cast<int>(marked.size()),
                                 0.78, 0.0, ImPlotSpec{ ImPlotProp_FillColor, operationColor(op->type) });
            }
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar();
    ImPlot::PopStyleColor(5);
}

} // namespace Sortly::ui
