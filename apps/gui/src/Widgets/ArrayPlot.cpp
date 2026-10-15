// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <array>
#include <cstddef>

namespace Sortly::ui {

void drawArrayPlot(const char* id, std::span<const int> data, const ArrayHighlight& highlight,
                   const ArrayPlotStyle& style, const ImVec2& size)
{
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,    ImVec4(0, 0, 0, 0));
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,     colors::withAlpha(colors::Clear, 0.55f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, colors::Border);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,   ImVec4(1, 1, 1, 0.06f));
    ImPlot::PushStyleColor(ImPlotCol_AxisText,   colors::Muted);
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, style.axes ? dp(14.0f, 12.0f) : dp(6.0f, 6.0f));

    const ImPlotFlags plotFlags = ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs;
    if (ImPlot::BeginPlot(id, size, plotFlags))
    {
        const ImPlotAxisFlags xFlags = style.axes ? ImPlotAxisFlags_NoGridLines : ImPlotAxisFlags_NoDecorations;
        const ImPlotAxisFlags yFlags = (style.axes ? ImPlotAxisFlags_None : ImPlotAxisFlags_NoDecorations) |
                                       ImPlotAxisFlags_AutoFit;
        ImPlot::SetupAxes(style.axes ? "Index" : nullptr, style.axes ? "Value" : nullptr, xFlags, yFlags);

        if (!data.empty())
        {
            const int maxValue = std::max(1, *std::max_element(data.begin(), data.end()));
            ImPlot::SetupAxesLimits(-0.5, static_cast<double>(data.size()) - 0.5,
                                    0.0, static_cast<double>(maxValue) * 1.1, ImPlotCond_Always);

            const ImVec4 base = highlight.sorted ? colors::withAlpha(colors::Success, 0.88f)
                                                 : colors::withAlpha(style.color, 0.88f);
            ImPlot::PlotBars("Values", data.data(), static_cast<int>(data.size()),
                             0.78, 0.0, ImPlotSpec{ ImPlotProp_FillColor, base });

            if (const SortOperation* op = highlight.operation; op && !highlight.sorted)
            {
                // Only the touched bars are repainted, with a light edge so they stand out on any base color.
                std::array<int, 2> xs{};
                std::array<int, 2> ys{};
                std::size_t count = 0;
                for (const int index : { op->indexA, op->indexB })
                {
                    if (index < 0 || static_cast<std::size_t>(index) >= data.size())
                        continue;
                    xs[count] = index;
                    ys[count] = data[static_cast<std::size_t>(index)];
                    ++count;
                }

                if (count > 0)
                {
                    ImPlot::PlotBars("Active", xs.data(), ys.data(), static_cast<int>(count), 0.78,
                                     ImPlotSpec{ ImPlotProp_FillColor, operationColor(op->type),
                                                 ImPlotProp_LineColor, colors::withAlpha(colors::TextBright, 0.9f) });
                }
            }
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar();
    ImPlot::PopStyleColor(5);
}

} // namespace Sortly::ui
