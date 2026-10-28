// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/BarMotion.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace Sortly::ui {

namespace {

constexpr double kBarWidth = 0.78;

// Every bar is in exactly one state. The order is also the draw order.
enum class BarState : std::size_t { Idle, Placed, Pivot, Active };

constexpr std::size_t kStateCount = 4;
constexpr std::array<const char*, kStateCount> kStateLabels = { "Idle", "Placed", "Pivot", "Active" };

// Bars of one state, in the column format ImPlot::PlotBars expects.
struct BarBatch
{
    std::vector<float> x;
    std::vector<float> y;

    void add(const ImVec2& bar) { x.push_back(bar.x); y.push_back(bar.y); }
    void clear()                { x.clear(); y.clear(); }
    [[nodiscard]] bool empty() const { return x.empty(); }
};

using BarBatches = std::array<BarBatch, kStateCount>;

// Buffers are reused between frames so drawing does not allocate. Plots are drawn one
// at a time on the UI thread, so one set is enough.
BarBatches& cleanBatches()
{
    static thread_local BarBatches batches;
    for (BarBatch& batch : batches)
        batch.clear();
    return batches;
}

[[nodiscard]] bool touchesSlot(const SortOperation* op, std::size_t slot)
{
    const int index = static_cast<int>(slot);
    return op && (op->indexA == index || op->indexB == index);
}

// Priority: active > pivot > in place > idle.
[[nodiscard]] BarState classify(std::size_t slot, std::span<const int> data, const ArrayHighlight& highlight)
{
    if (highlight.sorted)
        return BarState::Placed;
    if (touchesSlot(highlight.operation, slot))
        return BarState::Active;
    if (highlight.pivotIndex == static_cast<int>(slot))
        return BarState::Pivot;
    if (slot < highlight.target.size() && data[slot] == highlight.target[slot])
        return BarState::Placed;
    return BarState::Idle;
}

// Animated position when there is motion data, otherwise the bar sits on its own slot.
[[nodiscard]] ImVec2 barAt(const BarMotion* motion, std::span<const int> data, std::size_t slot)
{
    if (!motion)
        return ImVec2(static_cast<float>(slot), static_cast<float>(data[slot]));

    const std::size_t bar = motion->owner(slot);
    return ImVec2(motion->x(bar), motion->height(bar));
}

[[nodiscard]] ImVec4 fillColor(BarState state, const ArrayHighlight& highlight, const ArrayPlotStyle& style)
{
    switch (state)
    {
        case BarState::Idle:   return colors::withAlpha(colors::BarIdle, 0.9f);
        case BarState::Placed: return colors::withAlpha(highlight.sorted ? colors::Success : style.color, 0.88f);
        case BarState::Pivot:  return operationColor(SortOperation::Type::Pivot);
        case BarState::Active: return operationColor(highlight.operation->type); // Only reached with an operation.
    }
    return colors::Accent;
}

void drawBars(std::span<const int> data, const ArrayHighlight& highlight, const ArrayPlotStyle& style)
{
    // A motion of another size belongs to a previous dataset: ignore it.
    const BarMotion* motion = (highlight.motion && highlight.motion->size() == data.size()) ? highlight.motion : nullptr;

    BarBatches& batches = cleanBatches();
    for (std::size_t slot = 0; slot < data.size(); ++slot)
        batches[static_cast<std::size_t>(classify(slot, data, highlight))].add(barAt(motion, data, slot));

    for (std::size_t i = 0; i < kStateCount; ++i)
    {
        const BarBatch& batch = batches[i];
        if (batch.empty())
            continue;

        // No outline: bars are told apart by fill color only.
        const ImPlotSpec spec{ ImPlotProp_FillColor, fillColor(static_cast<BarState>(i), highlight, style),
                               ImPlotProp_LineColor, ImVec4(0, 0, 0, 0) };
        ImPlot::PlotBars(kStateLabels[i], batch.x.data(), batch.y.data(), static_cast<int>(batch.x.size()),
                         kBarWidth, spec);
    }
}

} // namespace

void drawArrayPlot(const char* id, std::span<const int> data, const ArrayHighlight& highlight,
                   const ArrayPlotStyle& style, const ImVec2& size)
{
    constexpr int kColorPushes = 5;
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,    ImVec4(0, 0, 0, 0));
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,     colors::withAlpha(colors::Clear, 0.55f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, colors::Border);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,   ImVec4(1, 1, 1, 0.06f));
    ImPlot::PushStyleColor(ImPlotCol_AxisText,   colors::Muted);
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, style.axes ? dp(14.0f, 12.0f) : dp(6.0f, 6.0f));

    if (ImPlot::BeginPlot(id, size, ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
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
            drawBars(data, highlight, style);
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar();
    ImPlot::PopStyleColor(kColorPushes);
}

} // namespace Sortly::ui
