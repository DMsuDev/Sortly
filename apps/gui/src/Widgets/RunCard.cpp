// Copyright (c) 2026 Dayron Mustelier (@DMsuDev)
// SPDX-License-Identifier: MIT

#include "Sortly/Gui/Widgets/RunCard.hpp"
#include "Sortly/Gui/Widgets/ArrayPlot.hpp"
#include "Sortly/Gui/Widgets/RunMetrics.hpp"
#include "Sortly/Gui/Widgets/UiKit.hpp"
#include "Sortly/Gui/Icons.hpp"
#include "Sortly/Gui/UiTheme.hpp"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>

namespace Sortly::ui {

namespace {

// Layout, in design pixels (scaled with dp()).
constexpr float kCardPaddingX      = 14.0f;
constexpr float kCardPaddingY      = 12.0f;
constexpr float kHeaderHeight      = 32.0f;
constexpr float kStatsWidth        = 112.0f;
constexpr float kPlotGap           = 10.0f;
constexpr float kFooterHeight      = 56.0f;
constexpr float kInlineStatsHeight = 40.0f;
constexpr float kInlineMinCell     = 84.0f;  // Narrower cells drop the last metrics.
constexpr float kMinPlotWidth      = 80.0f;
constexpr float kMinPlotHeight     = 60.0f;
constexpr float kViablePlotWidth   = 200.0f; // Below this a plot is too small to be useful.
constexpr float kViablePlotHeight  = 110.0f;
constexpr float kIdealPlotAspect   = 1.7f;   // Width / height the plot should get close to.
constexpr float kMinPlotAspect     = 1.35f;  // The card never grows taller than this allows.
constexpr float kInfoButtonSize    = 20.0f;
constexpr float kStatsPopupWidth   = 200.0f;
constexpr float kMinTextGap        = 8.0f;   // Least space kept between left and right text on a line.

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
    if (playback.isPlaying())  return { "RUNNING", colors::Info    };
    if (playback.isPaused())   return { "PAUSED",  colors::Warning };
    if (playback.isFinished()) return run.sweepDone() ? StateTag{ "SORTED",  colors::Success }
                                                      : StateTag{ "RUNNING", colors::Info    };
    return { "READY", colors::Muted };
}

// Dark card background with a faint tint of the slot's identity color.
ImVec4 cardBackground(const ImVec4& accent)
{
    constexpr float kTint = 0.03f;
    const ImVec4&   base  = colors::Surface;
    return ImVec4(base.x + (accent.x - base.x) * kTint,
                  base.y + (accent.y - base.y) * kTint,
                  base.z + (accent.z - base.z) * kTint, 1.0f);
}

// The statistics are narrow, so the longest label is shortened.
const char* shortLabel(Metric metric)
{
    return metric == Metric::ComputeTime ? "Time" : metricLabel(metric);
}

// Formatted value of a metric, or "-" while the run has no final statistics.
std::string metricText(const Run& run, Metric metric)
{
    return run.ready() ? formatMetric(run.session->getStatistics(), metric) : std::string("-");
}

// Width left on the current line after the last item (window-local, no scrolling in cards).
float spaceLeftOnLine()
{
    return ImGui::GetContentRegionMax().x - (ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x);
}

// Caption on the left, value pushed to the right edge.
void drawStatRow(const char* label, std::string_view value)
{
    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", label);
    ImGui::PopFont();

    rightAlignedOnLine(value, colors::TextBright);
}

void drawHeader(std::size_t slot, const Run& run, const ImVec4& color)
{
    const StateTag         state = stateOf(run);
    const std::string_view name  = SortAlgorithmFactory::toString(run.type);

    char title[64];
    std::snprintf(title, sizeof(title), "%zu. %.*s", slot + 1, static_cast<int>(name.size()), name.data());

    ImGui::PushFont(fonts::Heading);
    const float  titleHeight = ImGui::GetTextLineHeight();
    const float  dot         = dp(10.0f);
    const ImVec2 pos         = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(pos.x, pos.y + (titleHeight - dot) * 0.5f), ImVec2(pos.x + dot, pos.y + (titleHeight + dot) * 0.5f),
        toU32(color), dp(2.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dot + dp(8.0f));
    ImGui::TextUnformatted(title);
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
    highlight.sweep  = run.sweepProgress();

    if (playback.isStopped())
        return highlight;

    highlight.operation  = playback.getCurrentOperation();
    highlight.pivotIndex = playback.getPivotIndex();
    highlight.playback   = &playback;
    return highlight;
}

// ---------------------------------------------------------------------------
// Responsive body
// ---------------------------------------------------------------------------

enum class StatsLayout { Side, Inline, Compact };

struct BodyLayout
{
    StatsLayout stats;
    ImVec2      plot;
};

// Side or Inline when the plot stays readable (the most landscape one wins);
// otherwise the statistics move into a popup (Compact) and the plot gets all the space.
BodyLayout chooseLayout(const ImVec2& avail)
{
    const float inlineH = dp(kInlineStatsHeight) + ImGui::GetStyle().ItemSpacing.y;

    const ImVec2 side(std::max(dp(kMinPlotWidth),  avail.x - dp(kStatsWidth) - dp(kPlotGap)),
                      std::max(dp(kMinPlotHeight), avail.y - dp(kFooterHeight)));
    const ImVec2 below(std::max(dp(kMinPlotWidth),  avail.x),
                       std::max(dp(kMinPlotHeight), avail.y - dp(kFooterHeight) - inlineH));

    const auto viable   = [](const ImVec2& p) { return p.x >= dp(kViablePlotWidth) && p.y >= dp(kViablePlotHeight); };
    const auto distance = [](const ImVec2& p) { return std::abs(std::log(p.x / p.y / kIdealPlotAspect)); };

    const bool sideOk  = viable(side);
    const bool belowOk = viable(below);

    if (sideOk && (!belowOk || distance(side) <= distance(below)))
        return { StatsLayout::Side, side };
    if (belowOk)
        return { StatsLayout::Inline, below };

    return { StatsLayout::Compact,
             ImVec2(std::max(dp(kMinPlotWidth),  avail.x),
                    std::max(dp(kMinPlotHeight), avail.y - dp(kFooterHeight))) };
}

// Vertical list beside the plot. Rows that do not fit are dropped instead of making the card scroll.
void drawStatsColumn(const Run& run, float height)
{
    ImGui::BeginChild("##Stats", ImVec2(dp(kStatsWidth), height), ImGuiChildFlags_None,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::TextDim, "STATISTICS");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0.0f, dp(2.0f)));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dp(6.0f, 5.0f));
    for (const Metric metric : kMetrics)
    {
        if (ImGui::GetCursorPosY() + ImGui::GetTextLineHeight() > height)
            break;

        drawStatRow(shortLabel(metric), metricText(run, metric));
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

// One row under the plot: caption above, value below, one cell per metric.
// When the width does not fit every metric, the last ones are dropped.
void drawStatsInline(const Run& run)
{
    const float width   = ImGui::GetContentRegionAvail().x;
    const auto  columns = std::clamp<std::size_t>(static_cast<std::size_t>(width / dp(kInlineMinCell)), 1, kMetrics.size());
    const float cellW   = width / static_cast<float>(columns);

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList*  dl     = ImGui::GetWindowDrawList();

    for (std::size_t i = 0; i < columns; ++i)
    {
        const Metric      metric = kMetrics[i];
        const std::string value  = metricText(run, metric);
        const float       x      = origin.x + cellW * static_cast<float>(i);

        ImGui::PushFont(fonts::Label);
        dl->AddText(ImVec2(x, origin.y), toU32(colors::Muted), shortLabel(metric));
        const float captionH = ImGui::GetTextLineHeight();
        ImGui::PopFont();

        ImGui::PushFont(fonts::Body);
        dl->AddText(ImVec2(x, origin.y + captionH), toU32(colors::TextBright), value.c_str());
        ImGui::PopFont();
    }

    ImGui::Dummy(ImVec2(width, dp(kInlineStatsHeight)));
}

// Full statistics list, shared by the hover tooltip and the click popup.
void drawStatsContent(const Run& run)
{
    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::TextDim, "STATISTICS");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0.0f, dp(2.0f)));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dp(6.0f, 5.0f));
    for (const Metric metric : kMetrics)
        drawStatRow(shortLabel(metric), metricText(run, metric));
    drawStatRow("Complexity", run.session->getAlgorithmComplexity());
    ImGui::PopStyleVar();
}

// Small icon button at the right of the current line. Hover shows the statistics,
// click pins them in a popup (also works on touch screens).
void drawStatsButton(const Run& run, const ImVec4& color)
{
    constexpr const char* kPopupId = "##StatsPopup";
    const float size = dp(kInfoButtonSize);

    ImGui::SameLine(ImGui::GetContentRegionMax().x - size);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors::withAlpha(color, 0.18f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  colors::withAlpha(color, 0.30f));
    ImGui::PushStyleColor(ImGuiCol_Text,          color);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushFont(fonts::Label);
    const bool clicked = ImGui::Button(ICON_INFO "##StatsButton", ImVec2(size, size));
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    const bool   hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort);
    const ImVec2 anchor(ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y - dp(6.0f));

    if (clicked)
        ImGui::OpenPopup(kPopupId);

    // The button sits at the bottom of the card, so the panel opens above it.
    // SetNextWindow* only runs right before a Begin, otherwise it would leak into the next window.
    const bool pinned = ImGui::IsPopupOpen(kPopupId);
    if (!pinned && !hovered)
        return;

    ImGui::SetNextWindowPos(anchor, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(dp(kStatsPopupWidth), 0.0f));

    if (pinned)
    {
        if (beginPopup(kPopupId))
        {
            drawStatsContent(run);
            ImGui::EndPopup();
        }
    }
    else if (beginTooltip())
    {
        drawStatsContent(run);
        ImGui::EndTooltip();
    }
}

// Progress bar plus "N% · done / total steps" on the left and the complexity on the right.
// Compact cards swap the complexity for the statistics button (it is inside the popup).
void drawFooter(const Run& run, const ImVec4& color, bool compact)
{
    const bool  ready    = run.ready();
    const float progress = ready ? run.session->getPlayback().getProgress() : 0.0f;

    ImGui::Spacing();
    progressBar(progress, dp(6.0f), &color);

    char caption[96];
    if (ready)
    {
        const SortPlayback& playback = run.session->getPlayback();
        std::snprintf(caption, sizeof(caption), compact ? "%.0f%%  \xC2\xB7  %s / %s" : "%.0f%%  \xC2\xB7  %s / %s steps",
                      static_cast<double>(progress) * 100.0,
                      formatCount(playback.getCurrentOperationIndex()).c_str(),
                      formatCount(playback.getTotalOperations()).c_str());
    }
    else
    {
        std::snprintf(caption, sizeof(caption), compact ? "Building..." : "Building the operation timeline...");
    }

    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", caption);

    if (compact)
    {
        ImGui::PopFont();
        drawStatsButton(run, color);
        return;
    }

    // The complexity is secondary: it is dropped instead of overlapping a long caption.
    const std::string_view complexity = run.session->getAlgorithmComplexity();
    const char*            begin      = complexity.data();
    const float            needed     = ImGui::CalcTextSize(begin, begin + complexity.size()).x + dp(kMinTextGap);
    if (!complexity.empty() && spaceLeftOnLine() >= needed)
        rightAlignedOnLine(complexity, colors::TextDim);
    ImGui::PopFont();
}

void drawBody(const Run& run, const ImVec4& color)
{
    const BodyLayout layout = chooseLayout(ImGui::GetContentRegionAvail());

    const ArrayPlotStyle style{ color };
    drawArrayPlot("##Plot", run.session->getVisualData(), makeHighlight(run), style, layout.plot);

    switch (layout.stats)
    {
        case StatsLayout::Side:
            ImGui::SameLine(0.0f, dp(kPlotGap));
            drawStatsColumn(run, layout.plot.y);
            break;
        case StatsLayout::Inline:
            drawStatsInline(run);
            break;
        case StatsLayout::Compact:
            break;
    }

    drawFooter(run, color, layout.stats == StatsLayout::Compact);
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
    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBackground(color));
    ImGui::PushStyleColor(ImGuiCol_Border, colors::withAlpha(color, 0.55f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, dp(kCardPaddingX, kCardPaddingY));
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

float maxUsefulCardHeight(float cardWidth)
{
    const float plotW  = cardWidth - 2.0f * dp(kCardPaddingX);
    const float chrome = 2.0f * dp(kCardPaddingY) + dp(kHeaderHeight) + dp(kFooterHeight) + dp(kInlineStatsHeight);
    return plotW / kMinPlotAspect + chrome;
}

ImVec2 cardSideChrome()
{
    return dp(2.0f * kCardPaddingX + kStatsWidth + kPlotGap,
              2.0f * kCardPaddingY + kHeaderHeight + kFooterHeight);
}

} // namespace Sortly::ui
