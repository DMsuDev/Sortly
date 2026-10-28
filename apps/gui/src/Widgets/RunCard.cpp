// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/RunCard.hpp"
#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/Widgets/RunMetrics.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>

namespace Sortly::ui {

namespace {

// Layout, in design pixels (scaled with dp()).
constexpr float kStatsWidth    = 112.0f;
constexpr float kPlotGap       = 10.0f;
constexpr float kFooterHeight  = 56.0f;
constexpr float kMinPlotWidth  = 80.0f;
constexpr float kMinPlotHeight = 60.0f;
constexpr float kAxesMinWidth  = 520.0f; // Axes only when the plot is big enough to read them.
constexpr float kAxesMinHeight = 240.0f;

struct StateTag
{
    const char* text;
    ImVec4      color;
};

StateTag stateOf(const Run& run)
{
    if (!run.error.empty())
        return { "ERROR", colors::Danger };
    if (run.computing || !run.session)
        return { "BUILDING", colors::Warning };

    const SortPlayback& playback = run.session->getPlayback();
    if (playback.isPlaying())  return { "RUNNING", colors::Info };
    if (playback.isPaused())   return { "PAUSED",  colors::Warning };
    if (playback.isFinished()) return { "SORTED",  colors::Success };
    return { "READY", colors::Muted };
}

// The statistics column is narrow, so the longest label is shortened.
const char* shortLabel(Metric metric)
{
    return metric == Metric::ComputeTime ? "Time" : metricLabel(metric);
}

// Caption on the left, value pushed to the right edge.
void drawStatRow(const char* label, const std::string& value)
{
    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", label);
    ImGui::PopFont();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(value.c_str()).x);
    ImGui::TextColored(colors::TextBright, "%s", value.c_str());
}

// Text pushed to the right edge of the current line (no allocation: works on a view).
void rightAlignedDim(std::string_view text)
{
    const char* begin = text.data();
    const char* end   = begin + text.size();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(begin, end).x);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::TextDim);
    ImGui::TextUnformatted(begin, end);
    ImGui::PopStyleColor();
}

void drawHeader(std::size_t slot, const Run& run, const ImVec4& color)
{
    const StateTag state = stateOf(run);
    const std::string title = std::to_string(slot + 1) + ". " + std::string(SortAlgorithmFactory::toString(run.type));

    ImGui::PushFont(fonts::Heading);
    const float  titleHeight = ImGui::GetTextLineHeight();
    const float  dot         = dp(10.0f);
    const ImVec2 pos         = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(pos.x, pos.y + (titleHeight - dot) * 0.5f), ImVec2(pos.x + dot, pos.y + (titleHeight + dot) * 0.5f),
        ImGui::ColorConvertFloat4ToU32(color), dp(2.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dot + dp(8.0f));
    ImGui::TextUnformatted(title.c_str());
    ImGui::PopFont();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - pillWidth(state.text));
    pill(state.text, state.color);
}

// What the plot emphasizes. Nothing is emphasized until the run has started, so a fresh
// (Stopped) run shows every bar as idle instead of the first operation or chance matches.
ArrayHighlight makeHighlight(const Run& run)
{
    ArrayHighlight highlight;
    if (!run.ready())
        return highlight;

    const SortPlayback& playback = run.session->getPlayback();
    highlight.motion = &run.motion;
    highlight.sorted = playback.isFinished();

    if (playback.isStopped())
        return highlight;

    highlight.operation  = playback.getCurrentOperation();
    highlight.pivotIndex = playback.getPivotIndex();
    highlight.target     = run.target;
    return highlight;
}

void drawStatsColumn(const Run& run, float height)
{
    ImGui::BeginChild("##Stats", ImVec2(dp(kStatsWidth), height), ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::TextDim, "STATISTICS");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0.0f, dp(2.0f)));

    const bool ready = run.ready();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dp(6.0f, 5.0f));
    for (const Metric metric : kMetrics)
    {
        // Rows that do not fit are dropped instead of making the card scroll.
        if (ImGui::GetCursorPosY() + ImGui::GetTextLineHeight() > height)
            break;

        drawStatRow(shortLabel(metric), ready ? formatMetric(run.session->getStatistics(), metric) : "-");
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

// Progress bar plus "N% · done / total steps" on the left and the complexity on the right.
void drawFooter(const Run& run, const ImVec4& color)
{
    const bool  ready    = run.ready();
    const float progress = ready ? run.session->getPlayback().getProgress() : 0.0f;

    ImGui::Spacing();
    progressBar(progress, dp(6.0f), &color);

    char caption[96];
    if (ready)
    {
        const SortPlayback& playback = run.session->getPlayback();
        std::snprintf(caption, sizeof(caption), "%.0f%%  \xC2\xB7  %s / %s steps", static_cast<double>(progress) * 100.0,
                      formatCount(playback.getCurrentOperationIndex()).c_str(),
                      formatCount(playback.getTotalOperations()).c_str());
    }
    else
    {
        std::snprintf(caption, sizeof(caption), "Building the operation timeline...");
    }

    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", caption);
    rightAlignedDim(run.session->getAlgorithmComplexity());
    ImGui::PopFont();
}

void drawBody(const Run& run, const ImVec4& color)
{
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float  gap   = dp(kPlotGap);

    const float plotW = std::max(dp(kMinPlotWidth),  avail.x - dp(kStatsWidth) - gap);
    const float plotH = std::max(dp(kMinPlotHeight), avail.y - dp(kFooterHeight));

    const ArrayPlotStyle style{ color };
    drawArrayPlot("##Plot", run.session->getVisualData(), makeHighlight(run), style, layout.plot);

    ImGui::SameLine(0.0f, gap);
    drawStatsColumn(run, plotH);

    drawFooter(run, color);
}

void drawError(const std::string& message)
{
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
    ImGui::TextColored(colors::Danger, "%s", message.c_str());
    ImGui::PopTextWrapPos();
}

} // namespace

void drawRunCard(std::size_t slot, const Run& run, const ImVec2& size)
{
    const ImVec4 color = colors::slot(slot);

    ImGui::PushID(static_cast<int>(slot));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, colors::SurfaceHi);
    ImGui::PushStyleColor(ImGuiCol_Border, colors::withAlpha(color, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, dp(14.0f, 12.0f));
    ImGui::BeginChild("Card", size, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    // Soft accent glow in the top-left corner, drawn first so everything else sits on top.
    cornerGlow(ImGui::GetWindowPos(), std::min(size.x, size.y) * 0.9f, ImGui::GetStyle().ChildRounding, color, 0.16f);

    drawHeader(slot, run, color);
    ImGui::Spacing();

    if (!run.error.empty())
        drawError(run.error);
    else if (run.session)
        drawBody(run, color);

    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopID();
}

} // namespace Sortly::ui
