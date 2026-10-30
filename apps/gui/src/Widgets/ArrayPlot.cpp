// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/BarMotion.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include "Sortly/Sorting/Operations/SortPlayback.hpp"

#include <imgui.h>
#include <implot.h>

#include <algorithm>
#include <cstddef>
#include <vector>
#include <array>
#include <cmath>

namespace Sortly::ui {

namespace {

constexpr double kBarWidth = 0.78;

// Every bar is in exactly one state. The order is also the draw order.
enum class BarState : std::size_t { Idle, Placed, Pivot, Active, Sorted };

constexpr std::size_t kStateCount = 5;
constexpr std::array<const char*, kStateCount> kStateLabels = { "Idle", "Placed", "Pivot", "Active", "Sorted" };

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

// The data is settled, but the bar also has to have visually arrived at its slot.
[[nodiscard]] bool isPlaced(const BarMotion* motion, std::span<const int> data, std::size_t slot,
                            const ArrayHighlight& highlight)
{
    if (!highlight.playback || !highlight.playback->isSettled(slot))
        return false;
    if (!motion)
        return true;

    const std::size_t bar = motion->owner(slot);
    return std::abs(motion->x(bar) - static_cast<float>(slot)) < 0.05f &&
           std::abs(motion->height(bar) - static_cast<float>(data[slot])) < 0.5f;
}

// Priority: sorted sweep > active > pivot > in place > idle.
// Once the run is finished every bar is in place; the sweep decides which ones already turned green.
[[nodiscard]] BarState classify(std::size_t slot, std::size_t swept, std::span<const int> data,
                                const ArrayHighlight& highlight, const BarMotion* motion)
{
    if (highlight.sorted)
        return slot < swept ? BarState::Sorted : BarState::Placed;

    // Keep the pivot's identity color even when it participates in a comparison.
    if (highlight.pivotIndex == static_cast<int>(slot))
        return BarState::Pivot;

    if (touchesSlot(highlight.operation, slot))
        return BarState::Active;

    if (isPlaced(motion, data, slot, highlight))
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
        case BarState::Placed: return colors::withAlpha(style.color, 0.88f);
        case BarState::Pivot:  return operationColor(SortOperation::Type::Pivot);
        case BarState::Active: return operationColor(highlight.operation->type);
        case BarState::Sorted: return colors::withAlpha(colors::Success, 0.88f);
    }
    return colors::Accent;
}

void drawBars(std::span<const int> data, const ArrayHighlight& highlight, const ArrayPlotStyle& style)
{
    // A motion of another size belongs to a previous dataset: ignore it.
    const BarMotion* motion = (highlight.motion && highlight.motion->size() == data.size()) ? highlight.motion : nullptr;

    const float fraction = std::clamp(highlight.sweep, 0.0f, 1.0f);
    const auto  swept    = static_cast<std::size_t>(std::ceil(fraction * static_cast<float>(data.size())));

    BarBatches& batches = cleanBatches();
    for (std::size_t slot = 0; slot < data.size(); ++slot)
        batches[static_cast<std::size_t>(classify(slot, swept, data, highlight, motion))]
            .add(barAt(motion, data, slot));

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
    constexpr int   kColorPushes = 4;
    constexpr int   kGuideLines  = 5;
    ImPlot::PushStyleColor(ImPlotCol_FrameBg,    ImVec4(0, 0, 0, 0));
    ImPlot::PushStyleColor(ImPlotCol_PlotBg,     colors::withAlpha(colors::Clear, 0.80f));
    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, colors::Border);
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid,   ImVec4(1, 1, 1, 0.07f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, dp(6.0f, 6.0f));

    if (ImPlot::BeginPlot(id, size, ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
    {
        // No labels or ticks, but the Y grid stays as faint horizontal guides.
        constexpr ImPlotAxisFlags kNoText = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickMarks |
                                            ImPlotAxisFlags_NoTickLabels;
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, kNoText | ImPlotAxisFlags_AutoFit);

        if (!data.empty())
        {
            const double top = std::max(1, *std::max_element(data.begin(), data.end())) * 1.1;
            ImPlot::SetupAxesLimits(-0.5, static_cast<double>(data.size()) - 0.5, 0.0, top, ImPlotCond_Always);
            ImPlot::SetupAxisTicks(ImAxis_Y1, 0.0, top, kGuideLines);
            drawBars(data, highlight, style);
        }

        ImPlot::EndPlot();
    }

    ImPlot::PopStyleVar();
    ImPlot::PopStyleColor(kColorPushes);
}

} // namespace Sortly::ui
