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
#include <string>

namespace Sortly::ui {

namespace {

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

// Caption on the left, value pushed to the right edge.
void drawStatRow(const char* label, const std::string& value)
{
    ImGui::PushFont(fonts::Label);
    ImGui::TextColored(colors::Muted, "%s", label);
    ImGui::PopFont();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(value.c_str()).x);
    ImGui::TextColored(colors::TextBright, "%s", value.c_str());
}

struct CardStats
{
    std::string comparisons = "-";
    std::string swaps       = "-";
    std::string reads       = "-";
    std::string writes      = "-";
    std::string time        = "-";
    std::string complexity;
};

CardStats collectStats(const Run& run)
{
    CardStats out;
    if (!run.session)
        return out;

    out.complexity = std::string(run.session->getAlgorithmComplexity());
    if (!run.ready())
        return out;

    const SortStatistics& stats = run.session->getStatistics();
    out.comparisons = formatMetric(stats, Metric::Comparisons);
    out.swaps       = formatMetric(stats, Metric::Swaps);
    out.reads       = formatMetric(stats, Metric::Reads);
    out.writes      = formatMetric(stats, Metric::Writes);
    out.time        = formatMetric(stats, Metric::ComputeTime);
    return out;
}

void drawHeader(std::size_t slot, const Run& run, const ImVec4& color)
{
    const StateTag state = stateOf(run);
    const std::string title = std::to_string(slot + 1) + ". " + std::string(SortAlgorithmFactory::toString(run.type));

    ImGui::PushFont(fonts::Heading);
    const float titleHeight = ImGui::GetTextLineHeight();
    const float dot         = dp(10.0f);
    const ImVec2 pos        = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddRectFilled(
        ImVec2(pos.x, pos.y + (titleHeight - dot) * 0.5f), ImVec2(pos.x + dot, pos.y + (titleHeight + dot) * 0.5f),
        ImGui::ColorConvertFloat4ToU32(color), dp(2.0f));
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + dot + dp(8.0f));
    ImGui::TextUnformatted(title.c_str());
    ImGui::PopFont();

    ImGui::SameLine(ImGui::GetContentRegionMax().x - pillWidth(state.text));
    pill(state.text, state.color);
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

    drawHeader(slot, run, color);
    ImGui::Spacing();

    if (!run.error.empty())
    {
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(colors::Danger, "%s", run.error.c_str());
        ImGui::PopTextWrapPos();
    }
    else if (run.session)
    {
        const bool ready = run.ready();
        const CardStats stats = collectStats(run);

        const ImVec2 avail   = ImGui::GetContentRegionAvail();
        const float  statsW  = dp(112.0f);
        const float  gap     = dp(10.0f);
        const float  footerH = dp(56.0f);

        const float plotW = std::max(dp(80.0f), avail.x - statsW - gap);
        const float plotH = std::max(dp(60.0f), avail.y - footerH);

        ArrayHighlight highlight;
        if (ready)
        {
            const SortPlayback& playback = run.session->getPlayback();
            highlight.sorted     = playback.isFinished();
            highlight.operation  = playback.getCurrentOperation();
            highlight.pivotIndex = playback.getPivotIndex();
        }

        const ArrayPlotStyle style{ color, plotW > dp(520.0f) && plotH > dp(240.0f) };
        drawArrayPlot("##Plot", run.session->getVisualData(), highlight, style, ImVec2(plotW, plotH));

        ImGui::SameLine(0.0f, gap);
        ImGui::BeginChild("##Stats", ImVec2(statsW, plotH), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PushFont(fonts::Label);
        ImGui::TextColored(colors::TextDim, "STATISTICS");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(0.0f, dp(2.0f)));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, dp(6.0f, 5.0f));
        const struct { const char* label; const std::string* value; } rows[] = {
            { "Comparisons", &stats.comparisons }, { "Swaps", &stats.swaps }, { "Reads", &stats.reads },
            { "Writes", &stats.writes }, { "Time", &stats.time },
        };
        for (const auto& row : rows)
        {
            // Rows that do not fit are dropped instead of making the card scroll.
            if (ImGui::GetCursorPosY() + ImGui::GetTextLineHeight() > plotH)
                break;
            drawStatRow(row.label, *row.value);
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::Spacing();
        const float progress = ready ? run.session->getPlayback().getProgress() : 0.0f;
        progressBar(progress, dp(6.0f), &color);

        char left[96];
        if (ready)
        {
            const SortPlayback& playback = run.session->getPlayback();
            std::snprintf(left, sizeof(left), "%.0f%%  \xC2\xB7  %s / %s steps", static_cast<double>(progress) * 100.0,
                          formatCount(playback.getCurrentOperationIndex()).c_str(),
                          formatCount(playback.getTotalOperations()).c_str());
        }
        else
        {
            std::snprintf(left, sizeof(left), "Building the operation timeline...");
        }

        ImGui::PushFont(fonts::Label);
        ImGui::TextColored(colors::Muted, "%s", left);
        if (!stats.complexity.empty())
        {
            ImGui::SameLine(ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(stats.complexity.c_str()).x);
            ImGui::TextColored(colors::TextDim, "%s", stats.complexity.c_str());
        }
        ImGui::PopFont();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopID();
}

} // namespace Sortly::ui
